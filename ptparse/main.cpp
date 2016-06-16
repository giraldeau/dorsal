/* Intel PT Extractor
 * ------------------
 *
 * (C) 2016 DORSAL Lab
 *
 * Francis Giraldeau <francis.giraldeau@gmail.com>
 * Suchakra Sharma <suchakrapani.sharma@polymtl.ca>
 *
 * Goes through the perf.data file for which the intel_pt PMU was used
 * at record time. The recorded raw PT data is then extracted from the
 * AUX buffers where perf had saved it.
 *
 * DISCLAIMER : Experimental software. Handle with care.
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QtEndian>
#include <QDebug>

#include <linux/perf_event.h>

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define BIT(nr)			(1UL << (nr))
#define BIT_ULL(nr)		(1ULL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)	(1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)	((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE		8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define DECLARE_BITMAP(name,bits) \
    unsigned long name[BITS_TO_LONGS(bits)]

#define HEADER_FEAT_BITS 256

typedef qint64 u64;
typedef qint32 u32;

enum perf_user_event_type { /* above any possible kernel type */
    PERF_RECORD_USER_TYPE_START		= 64,
    PERF_RECORD_HEADER_ATTR			= 64,
    PERF_RECORD_HEADER_EVENT_TYPE		= 65, /* depreceated */
    PERF_RECORD_HEADER_TRACING_DATA		= 66,
    PERF_RECORD_HEADER_BUILD_ID		= 67,
    PERF_RECORD_FINISHED_ROUND		= 68,
    PERF_RECORD_ID_INDEX			= 69,
    PERF_RECORD_AUXTRACE_INFO		= 70,
    PERF_RECORD_AUXTRACE			= 71,
    PERF_RECORD_AUXTRACE_ERROR		= 72,
    PERF_RECORD_HEADER_MAX
};

struct perf_file_section {
    u64 offset;
    u64 size;
};

struct perf_file_header {
    u64				magic;
    u64				size;
    u64				attr_size;
    struct perf_file_section	attrs;
    struct perf_file_section	data;
    /* event_types is ignored */
    struct perf_file_section	event_types;
    DECLARE_BITMAP(adds_features, HEADER_FEAT_BITS);
};

struct auxtrace_event {
    struct perf_event_header header;
    u64 size;
    u64 offset;
    u64 reference;
    u32 idx;
    u32 tid;
    u32 cpu;
    u32 reserved__; /* For alignment */
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ptparse");
    QCoreApplication::setApplicationVersion("0.1");
    QCommandLineParser parser;

    parser.setApplicationDescription("Extract Intel PT data from perf.data AUX buffer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("perf.data", "path to perf.data file");

    parser.process(app);
    const QStringList args = parser.positionalArguments();

    QString perfData = "perf.data";
    QString ptFileName = "extracted.pt";

    /* Custom path, if needed */
    if (args.size() > 0) {
        perfData = args.at(0);  /* Input perf.data file */
        ptFileName = args.at(1);    /* Extracted PT file */
    }

    QFile file(perfData);
    if (file.open(QFile::ReadOnly)) {
        uchar *buf = file.map(0, file.size());
        QString str(QByteArray{(char *)buf, 8});

        struct perf_file_header* perfHead;
        perfHead = (struct perf_file_header*) buf;

        //qDebug() << QString("%1").arg(perfHead->data.offset, 0, 16);
        //qDebug() << QString("%1").arg(perfHead->data.size, 0, 16);

        int pos = perfHead->data.offset;

        fflush(stderr);

        int prev_pos = 0;
        //qDebug() << "file size:" << file.size();

        while (pos < file.size()) {

            struct perf_event_header *h = (struct perf_event_header *) (buf + pos);

            switch(h->type) {

            case PERF_RECORD_AUXTRACE:
            {
                struct auxtrace_event *aux = (struct auxtrace_event *) (buf + pos);

                if (aux->size == 0)
                    break;

                QByteArray baba;

                /* Skip 48 bytes of auxtrace_event struct also */
                char *ptData = (char *) (buf + pos) + sizeof(struct auxtrace_event);

                baba.setRawData(ptData, aux->size);
                QString extn = QString::number(aux->cpu); /* Extension is AUX buf's CPU */

                /* Write per-CPU files */
                QFile ptFile(ptFileName + "." + extn);
                //qDebug() << ptFile.fileName();
                ptFile.open(QIODevice::WriteOnly | QIODevice::Append);
                ptFile.write(baba);
                ptFile.close();

                /* Advance pos by AUX size */
                pos += aux->size;

                /* Advance pos by event size */
                pos += h->size;
            }
                break;

            default:
                /* Advance pos by event size */
                pos += h->size;
                break;
            }

            /* TODO: Investigate further and make sure what the extra bytes are just before header
             * starts. Refer https://twitter.com/tuxology/status/742701706572668928
             *
             * If event size is 0, assume we are not an event and bail out. Usually always happens
             * just after some PERF_RECORD_FINISHED_ROUND events right before the perf header at
             * the end of perf.data
             */

            if (prev_pos == pos)
                return 0;
            prev_pos = pos;

        }
    }
    return 0;
}
