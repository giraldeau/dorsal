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
#include <libdwarf.h>

int show_ehframe(uchar *buf, uint size)
{
    if (memcmp(buf, ELFMAG, SELFMAG) != 0) {
        printf("magic number not found");
        return -1;
    }

    uchar cls = buf[EI_CLASS];
    if (cls == 1) {
        printf("cannot parse 32-bit ELF");
        return 0;
    }

    uchar order = buf[EI_DATA];
    if (order == 2) {
        printf("cannot parse big endian");
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
            printf("off=0x%lx filesz=0x%lx elf_size=0x%x\n",
                   current->p_offset, current->p_filesz, size);
            uint end = current->p_offset + current->p_filesz;
            if (end >= size) {
                printf("section is beyond end of file");
                continue;
            }
            // parse the eh_frame sections
        }
    }
    return 0;
}

/*
 * Example taken from libdwarf
 */

#define UNDEF_VAL 2000
#define SAME_VAL 2001
#define CFA_VAL 2002

static void
print_reg(int r)
{
    switch(r) {
    case SAME_VAL:
        printf(" %d SAME_VAL ",r);
        break;
    case UNDEF_VAL:
        printf(" %d UNDEF_VAL ",r);
        break;
    case CFA_VAL:
        printf(" %d (CFA) ",r);
        break;
    default:
        printf(" r%d ",r);
        break;
    }
}

static void
print_one_regentry(const char *prefix, struct Dwarf_Regtable_Entry3_s *entry)
{
    int is_cfa = !strcmp("cfa",prefix);
    printf("%s ",prefix);
    printf("type: %d %s ",
        entry->dw_value_type,
        (entry->dw_value_type == DW_EXPR_OFFSET)? "DW_EXPR_OFFSET":
        (entry->dw_value_type == DW_EXPR_VAL_OFFSET)? "DW_EXPR_VAL_OFFSET":
        (entry->dw_value_type == DW_EXPR_EXPRESSION)? "DW_EXPR_EXPRESSION":
        (entry->dw_value_type == DW_EXPR_VAL_EXPRESSION)?
            "DW_EXPR_VAL_EXPRESSION":
            "Unknown");

    switch(entry->dw_value_type) {
    case DW_EXPR_OFFSET:
        print_reg(entry->dw_regnum);
        printf(" offset_rel? %d ",entry->dw_offset_relevant);
        if(entry->dw_offset_relevant) {
            printf(" offset  %" DW_PR_DSd " " ,
                   entry->dw_offset_or_block_len);
            if(is_cfa) {
                printf("defines cfa value");
            } else {
                printf("address of value is CFA plus signed offset");
            }
            if(!is_cfa  && entry->dw_regnum != CFA_VAL) {
                printf(" compiler botch, regnum != CFA_VAL");
            }
        } else {
            printf("value in register");
        }
        break;

    case DW_EXPR_VAL_OFFSET:
        print_reg(entry->dw_regnum);
        printf(" offset  %" DW_PR_DSd " " ,
               entry->dw_offset_or_block_len);
        if(is_cfa) {
            printf("does this make sense? No?");
        } else {
            printf("value at CFA plus signed offset");
        }
        if(!is_cfa  && entry->dw_regnum != CFA_VAL) {
            printf(" compiler botch, regnum != CFA_VAL");
        }
        break;
    case DW_EXPR_EXPRESSION:
        print_reg(entry->dw_regnum);
        printf(" offset_rel? %d ",entry->dw_offset_relevant);
        printf(" offset  %" DW_PR_DSd " " ,
               entry->dw_offset_or_block_len);
        printf("Block ptr set? %s ",entry->dw_block_ptr?"yes":"no");
        printf(" Value is at address given by expr val ");
        /* printf(" block-ptr  0x%" DW_PR_DUx " ",
        (Dwarf_Unsigned)entry->dw_block_ptr); */
        break;
    case DW_EXPR_VAL_EXPRESSION:
        printf(" expression byte len  %" DW_PR_DSd " " ,
               entry->dw_offset_or_block_len);
        printf("Block ptr set? %s ",entry->dw_block_ptr?"yes":"no");
        printf(" Value is expr val ");
        if(!entry->dw_block_ptr) {
            printf("Compiler botch. ");
        }
        /* printf(" block-ptr  0x%" DW_PR_DUx " ",
        (Dwarf_Unsigned)entry->dw_block_ptr); */
        break;
    }
    printf("\n");
}

static void
print_regtable(Dwarf_Regtable3 *tab3)
{
    int r;
    print_one_regentry("cfa", &tab3->rt3_cfa_rule);

    for(r = 0; r < tab3->rt3_reg_table_size; r++) {
        char rn[30];
        snprintf(rn, sizeof(rn), "reg %d", r);
        print_one_regentry(rn, tab3->rt3_rules + r);
    }
}

static void
print_frame_instrs(Dwarf_Frame_Op *frame_op_list,
                   Dwarf_Signed frame_op_count)
{
    Dwarf_Signed i = 0;
    printf("%-12s %-8s %-8s %-8s %-8s\n", "Base op.", "Ext op.", "Reg.", "Offset.", "Instr-offset.");
    for(i = 0; i < frame_op_count; ++i) {
        printf("[%-10" DW_PR_DSd "]", i);
        printf(" %-7d ", frame_op_list[i].fp_base_op);
        printf(" %-7d ", frame_op_list[i].fp_extended_op);
        printf(" %-8" DW_PR_DSd , frame_op_list[i].fp_offset);
        printf(" 0x%-8" DW_PR_DUx, frame_op_list[i].fp_instr_offset);
        printf("\n");
    }
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
            std::cout << "failed to open file " << path.toStdString() << std::endl;
            continue;
        }

        uchar *buf = elf.map(0, elf.size());
        if (buf == 0) {
            std::cout << "failed to map file" << path.toStdString() << std::endl;
        }
        if (show_ehframe(buf, elf.size()) < 0) {
            std::cout << "failed to parse eh_frame" << std::endl;
        }

        /* TODO: look at how libdwarf handles multiarch */
        Dwarf_Debug dbg;
        int res = dwarf_init(elf.handle(), DW_DLC_READ, nullptr, nullptr, &dbg, nullptr);
        if (res != DW_DLV_OK) {
            printf("dwarf_init error: %d\n", res);
            continue;
        }

        Dwarf_Cie *cie_data;
        Dwarf_Signed cie_count;
        Dwarf_Fde *fde_data;
        Dwarf_Signed fde_count;

        res = dwarf_get_fde_list_eh(dbg, &cie_data, &cie_count, &fde_data, &fde_count, nullptr);

        if (res == DW_DLV_OK) {

            std::cout << "cie_count: " << cie_count << std::endl;
            std::cout << "fde_count: " << fde_count << std::endl;

            for (int i = 0; i < fde_count; i++) {
                Dwarf_Fde fde = fde_data[i];
                Dwarf_Cie cie;
                dwarf_get_cie_of_fde(fde, &cie, nullptr);



                Dwarf_Unsigned cie_size;
                Dwarf_Small version;
                char *aug;
                Dwarf_Unsigned code_align;
                Dwarf_Signed data_align;
                Dwarf_Half ret_addr_reg_rule;
                Dwarf_Ptr instructions;
                Dwarf_Unsigned instructions_length;
                dwarf_get_cie_info(cie, &cie_size, &version, &aug,
                                   &code_align, &data_align, &ret_addr_reg_rule,
                                   &instructions, &instructions_length, nullptr);
                std::cout << "cie: " << cie << " fde: " << fde  << std::endl;
                std::cout << "cie_info:" << cie_size << " " << version << " " << aug <<
                            " " << code_align << " " << data_align << " " << ret_addr_reg_rule <<
                            instructions << " " << instructions_length  << " " << std::endl;

                Dwarf_Addr low_pc;
                Dwarf_Unsigned func_length;
                Dwarf_Ptr fde_bytes;
                Dwarf_Unsigned fde_size;
                Dwarf_Off cie_offset;
                Dwarf_Signed cie_index;
                Dwarf_Off fde_offset;
                dwarf_get_fde_range(fde, &low_pc, &func_length, &fde_bytes, &fde_size,
                                    &cie_offset, &cie_index, &fde_offset, nullptr);
                std::cout << "fde_range: " << low_pc << " " << func_length << std::endl;

                Dwarf_Half old_rule_count = dwarf_set_frame_rule_table_size(dbg, 1);
                dwarf_set_frame_rule_table_size(dbg, old_rule_count);

                std::cout << "rule_count: " << old_rule_count << std::endl;

                /* Iterate over instructions */
                Dwarf_Addr pc = low_pc + (func_length / 2); // dummy address
                Dwarf_Regtable3 reg_table;
                Dwarf_Addr row_pc;

                reg_table.rt3_reg_table_size = old_rule_count;
                uint reg_table_size = sizeof(struct Dwarf_Regtable_Entry3_s) * old_rule_count;
                reg_table.rt3_rules = (struct Dwarf_Regtable_Entry3_s *)malloc(reg_table_size);
                memset(reg_table.rt3_rules, 0, reg_table_size);

                dwarf_get_fde_info_for_all_regs3(fde, pc, &reg_table, &row_pc, nullptr);

                /* a bit of nonsense */
                //print_regtable(fde, &reg_table, old_rule_count, dbg, nullptr);

                Dwarf_Ptr outinstrs;
                Dwarf_Unsigned instrslen;
                res = dwarf_get_fde_instr_bytes(fde, &outinstrs, &instrslen, nullptr);
                if (res != DW_DLV_OK) {
                    printf("error dwarf_get_fde_instr_bytes\n");
                    continue;
                }

                Dwarf_Frame_Op *frame_op_list;
                Dwarf_Signed frame_op_count;
                res = dwarf_expand_frame_instructions(cie, outinstrs, instrslen, &frame_op_list, &frame_op_count, nullptr);
                if(res != DW_DLV_OK) {
                    printf("dwarf_expand_frame_instructions failed!\n");
                    continue;
                }

                printf("Frame op count: %" DW_PR_DUu "\n", frame_op_count);
                print_frame_instrs(frame_op_list, frame_op_count);

                dwarf_dealloc(dbg, frame_op_list, DW_DLA_FRAME_BLOCK);
                free(reg_table.rt3_rules);

            }

            dwarf_fde_cie_list_dealloc(dbg, cie_data, cie_count, fde_data, fde_count);
        }

        dwarf_finish(dbg, nullptr);
    }
    return 0;
}

