#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <QtEndian>


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

struct perf_file_section {
    u64 offset;
    u64 size;
}__attribute__((packed));

/*size is (u64*3) + (3*(u64*2)) + 4? */
/* 24  + 48 + 4? = 76 bytes */
struct perf_file_header {
    u64				magic;
    u64				size;
    u64				attr_size;
    struct perf_file_section	attrs;
    struct perf_file_section	data;
    /* event_types is ignored */
    struct perf_file_section	event_types;
    DECLARE_BITMAP(adds_features, HEADER_FEAT_BITS);
}__attribute__((packed));

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ptparse");
    QCoreApplication::setApplicationVersion("0.1");
    QCommandLineParser parser;

    parser.setApplicationDescription("extract intel pt buffers from perf.data");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("perf.data", "path to perf.data file");

    parser.process(app);
    const QStringList args = parser.positionalArguments();

    qDebug() << args;
    QString perfData = "perf.data";
    if (args.size() > 0) {
        perfData = args.at(0);
    }

    qDebug() << perfData;

    QFile file(perfData);
    if (file.open(QFile::ReadOnly)) {
        qDebug() << "file size:" << file.size();
        uchar *buf = file.map(0, file.size());

        QString str(QByteArray{(char *)buf, 8});
        qDebug() << str;

        struct perf_file_header* perfHead;
        perfHead = (struct perf_file_header*) buf; //probably a good way is to memcpy properly


        qDebug() << "0x" + QString::number(qToBigEndian(perfHead->magic), 16); // we need to change byteorder

        qDebug() << "0x" + QString::number(qToBigEndian(perfHead->size), 16);
        qDebug() << "0x" + QString::number(sizeof(*perfHead), 16);

        qDebug() << "0x" + QString::number(qToBigEndian(perfHead->attr_size), 16);

        qDebug() << "0x" + QString::number(qToBigEndian((u64)perfHead->adds_features[0]), 16);

        int pos = perfHead->data.offset;

        qDebug() << perfHead->data.offset << QString("%1").arg(perfHead->data.offset, 0, 16);
        qDebug() << perfHead->data.size << QString("%1").arg(perfHead->data.size, 0, 16);

        fflush(stderr);

        while (pos < file.size()) {
            uchar *x = buf + pos;
            ushort *blah = (ushort *) x;
            qDebug() << *blah;

            struct perf_event_header *h = (struct perf_event_header *) (buf + pos);
            printf("type 0x%x\n", h->type);
            printf("size %d\n", h->size);
            pos += h->size;
            break;
        }

    }


    return 0;
}

