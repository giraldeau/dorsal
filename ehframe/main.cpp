/*
 * Parse ELF info and show eh_frame section
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QByteArray>
#include <QBuffer>

#include <elf.h>
#include <iostream>
#include <cstring>
#include <cstdio>

int show_ehframe(uchar *buf, uint size)
{
    if (memcmp(buf, ELFMAG, SELFMAG) != 0) {
        qDebug() << "magic number not found";
        return -1;
    }

    uchar cls = buf[EI_CLASS];
    if (cls == 1) {
        qDebug() << "cannot parse 32-bit ELF";
        return 0;
    }

    uchar order = buf[EI_DATA];
    if (order == 2) {
        qDebug() << "cannot parse big endian";
    }

    Elf64_Ehdr *hdr = (Elf64_Ehdr *) buf;
    Elf64_Phdr *phdr = (Elf64_Phdr *) (buf + hdr->e_phoff);
    for (int i = 0; i < hdr->e_phnum; i++) {
        std::string name = "unknown";
        Elf64_Phdr *current = &phdr[i];
        Elf64_Word type = current->p_type;
        switch(type) {
        case PT_NULL:
            name = "PT_NULL";
            break;
        case PT_LOAD:
            name = "PT_LOAD";
            break;
        case PT_DYNAMIC:
            name = "PT_DYNAMIC";
            break;
        case PT_INTERP:
            name = "PT_INTERP";
            break;
        case PT_NOTE:
            name = "PT_NOTE";
            break;
        case PT_SHLIB:
            name = "PT_SHLIB";
            break;
        case PT_PHDR:
            name = "PT_PHDR";
            break;
        case PT_TLS:
            name = "PT_TLS";
            break;
        case PT_NUM:
            name = "PT_NUM";
            break;
        case PT_LOOS:
            name = "PT_LOOS";
            break;
        case PT_GNU_EH_FRAME:
            name = "PT_GNU_EH_FRAME";
            break;
        case PT_GNU_STACK:
            name = "PT_GNU_STACK";
            break;
        case PT_GNU_RELRO:
            name = "PT_GNU_RELRO";
            break;
        default:
            break;
        }
        std::cout << "phdr type:" << name << std::endl;

        if (type == PT_GNU_EH_FRAME) {
            printf("off=0x%llx filesz=0x%llx elf_size=0x%llx\n", current->p_offset, current->p_filesz, size);
            uint end = current->p_offset + current->p_filesz;
            if (end >= size) {
                qDebug() << "section is beyond end of file";
                continue;
            }

            // parse the eh_frame sections


        }

    }

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("elf", "path to ELF file");

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    QListIterator<QString> iter(args);
    while (iter.hasNext()) {
        const QString path = iter.next();
        QFile elf(path);
        if (!elf.open(QIODevice::ReadOnly)) {
            qDebug() << "failed to open " << path;
            continue;
        }
        uchar *buf = elf.map(0, elf.size());
        if (buf == 0) {
            qDebug() << "failed to map file" << path;
        }
        if (show_ehframe(buf, elf.size()) < 0) {
            qDebug() << "failed to parse eh_frame";
        }
    }
    return 0;
}

