#include <QDebug>
#include <QFile>
#include <QMap>
#include <QDataStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <cstdio>

#include "stdio.h"
#include "unistd.h"
#include "inttypes.h"

uintptr_t vtop(uintptr_t vaddr) {
    FILE *pagemap;
    intptr_t paddr = 0;
    uint64_t offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
    uint64_t e;

    // https://www.kernel.org/doc/Documentation/vm/pagemap.txt
    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e, sizeof(uint64_t), 1, pagemap)) {
                if (e & (1ULL << 63)) { // page present ?
                    paddr = e & ((1ULL << 54) - 1); // pfn mask
                    paddr = paddr * sysconf(_SC_PAGESIZE);
                    // add offset within page
                    paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
                }
            }
        }
        fclose(pagemap);
    }

    return paddr;
}

void hexdump(uchar *buf, int length)
{
    int rows = length / 8;
    for (int row = 0; row < rows; row++) {
        int off = row * 8;
        printf("%08x ", off);
        for (int i = off; i < off + 8; i++) {
            printf("%02x ", buf[i]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    QString fn = "try_to_wake_up";

    int length = 64;

    QFile kallsyms("/proc/kallsyms");

    QMap<QString, ulong> syms;

    if (kallsyms.open(QFile::ReadOnly)) {
        QByteArray buf = kallsyms.readAll();
        qDebug() << buf.size();
        QTextStream in(&buf);
        QRegularExpression re("(?<addr>[a-fA-F0-9]+)[\\s]*(?<type>\\w+)[\\s]*(?<sym>\\w+)[\\s]*(?<mod>\\[\\w+\\])?");
        if (!re.isValid()) {
            qDebug() << "regex is invalid";
            exit(1);
        }
        while (!in.atEnd()) {
            QString line = in.readLine();
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                //qDebug() << match.captured("addr") << match.captured("type") << match.captured("sym");

                if (match.captured("sym").compare(fn) == 0) {
                    qDebug() << "found symbol" << match.captured("sym");
                }

                if (match.captured("type").compare("t", Qt::CaseInsensitive) == 0) {
                    ulong addr = match.captured("addr").toULong(nullptr, 16);
                    syms.insert(match.captured("sym"), addr);



                }
            }
        }
    }

    qDebug() << syms.size();

    if (!syms.contains(fn)) {
        qDebug() << "symbol not found" << fn;
    }

    ulong symoff = syms.find(fn).value();

    qDebug() << "symoff" << symoff;

    printf("%s %lx\n", fn.toStdString().c_str(), symoff);

    ulong addr = vtop(symoff);
    printf("phys %lx\n", addr);
    QFile mem("/dev/mem");
    mem.open(QFile::ReadOnly);

    uchar *buf = mem.map(addr, length);

    qDebug() << buf;

    if (buf) {
        hexdump(buf, length);
    }

    return 0;
}

