#include <fstream>
#include <iostream>
#include <iomanip>
#include <uuid/uuid.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "utils.hpp"
#include "scalar.hpp"
#include "pols.hpp"
#include <openssl/md5.h>
#include "merkle.hpp"

using namespace std;

void printRegs(Context &ctx)
{
    cout << "Registers:" << endl;
    printReg(ctx, "A7", ctx.pols.A7[*ctx.pStep]);
    printReg(ctx, "A6", ctx.pols.A6[*ctx.pStep]);
    printReg(ctx, "A5", ctx.pols.A5[*ctx.pStep]);
    printReg(ctx, "A4", ctx.pols.A4[*ctx.pStep]);
    printReg(ctx, "A3", ctx.pols.A3[*ctx.pStep]);
    printReg(ctx, "A2", ctx.pols.A2[*ctx.pStep]);
    printReg(ctx, "A1", ctx.pols.A1[*ctx.pStep]);
    printReg(ctx, "A0", ctx.pols.A0[*ctx.pStep]);
    printReg(ctx, "B7", ctx.pols.B7[*ctx.pStep]);
    printReg(ctx, "B6", ctx.pols.B6[*ctx.pStep]);
    printReg(ctx, "B5", ctx.pols.B5[*ctx.pStep]);
    printReg(ctx, "B4", ctx.pols.B4[*ctx.pStep]);
    printReg(ctx, "B3", ctx.pols.B3[*ctx.pStep]);
    printReg(ctx, "B2", ctx.pols.B2[*ctx.pStep]);
    printReg(ctx, "B1", ctx.pols.B1[*ctx.pStep]);
    printReg(ctx, "B0", ctx.pols.B0[*ctx.pStep]);
    printReg(ctx, "C7", ctx.pols.C7[*ctx.pStep]);
    printReg(ctx, "C6", ctx.pols.C6[*ctx.pStep]);
    printReg(ctx, "C5", ctx.pols.C5[*ctx.pStep]);
    printReg(ctx, "C4", ctx.pols.C4[*ctx.pStep]);
    printReg(ctx, "C3", ctx.pols.C3[*ctx.pStep]);
    printReg(ctx, "C2", ctx.pols.C2[*ctx.pStep]);
    printReg(ctx, "C1", ctx.pols.C1[*ctx.pStep]);
    printReg(ctx, "C0", ctx.pols.C0[*ctx.pStep]);
    printReg(ctx, "D7", ctx.pols.D7[*ctx.pStep]);
    printReg(ctx, "D6", ctx.pols.D6[*ctx.pStep]);
    printReg(ctx, "D5", ctx.pols.D5[*ctx.pStep]);
    printReg(ctx, "D4", ctx.pols.D4[*ctx.pStep]);
    printReg(ctx, "D3", ctx.pols.D3[*ctx.pStep]);
    printReg(ctx, "D2", ctx.pols.D2[*ctx.pStep]);
    printReg(ctx, "D1", ctx.pols.D1[*ctx.pStep]);
    printReg(ctx, "D0", ctx.pols.D0[*ctx.pStep]);
    printReg(ctx, "E7", ctx.pols.E7[*ctx.pStep]);
    printReg(ctx, "E6", ctx.pols.E6[*ctx.pStep]);
    printReg(ctx, "E5", ctx.pols.E5[*ctx.pStep]);
    printReg(ctx, "E4", ctx.pols.E4[*ctx.pStep]);
    printReg(ctx, "E3", ctx.pols.E3[*ctx.pStep]);
    printReg(ctx, "E2", ctx.pols.E2[*ctx.pStep]);
    printReg(ctx, "E1", ctx.pols.E1[*ctx.pStep]);
    printReg(ctx, "E0", ctx.pols.E0[*ctx.pStep]);
    printReg(ctx, "SR7", ctx.pols.SR7[*ctx.pStep]);
    printReg(ctx, "SR6", ctx.pols.SR6[*ctx.pStep]);
    printReg(ctx, "SR5", ctx.pols.SR5[*ctx.pStep]);
    printReg(ctx, "SR4", ctx.pols.SR4[*ctx.pStep]);
    printReg(ctx, "SR3", ctx.pols.SR3[*ctx.pStep]);
    printReg(ctx, "SR2", ctx.pols.SR2[*ctx.pStep]);
    printReg(ctx, "SR1", ctx.pols.SR1[*ctx.pStep]);
    printReg(ctx, "SR0", ctx.pols.SR0[*ctx.pStep]);
    printReg(ctx, "CTX", ctx.pols.CTX[*ctx.pStep]);
    printReg(ctx, "SP", ctx.pols.SP[*ctx.pStep]);
    printReg(ctx, "PC", ctx.pols.PC[*ctx.pStep]);
    printReg(ctx, "MAXMEM", ctx.pols.MAXMEM[*ctx.pStep]);
    printReg(ctx, "GAS", ctx.pols.GAS[*ctx.pStep]);
    printReg(ctx, "zkPC", ctx.pols.zkPC[*ctx.pStep]);
    Goldilocks::Element step;
    step = ctx.fr.fromU64(*ctx.pStep);
    printReg(ctx, "STEP", step, false, true);
#ifdef LOG_FILENAME
    cout << "File: " << ctx.fileName << " Line: " << ctx.line << endl;
#endif
}

void printVars(Context &ctx)
{
    cout << "Variables:" << endl;
    uint64_t i = 0;
    for (map<string, Goldilocks::Element>::iterator it = ctx.vars.begin(); it != ctx.vars.end(); it++)
    {
        cout << "i: " << i << " varName: " << it->first << " fe: " << ctx.fr.toU64(it->second) << endl;
        i++;
    }
}

string printFea(Context &ctx, Fea &fea)
{
    return "fe0:" + ctx.fr.toString(fea.fe0, 16) +
           " fe1:" + ctx.fr.toString(fea.fe1, 16) +
           " fe2:" + ctx.fr.toString(fea.fe2, 16) +
           " fe3:" + ctx.fr.toString(fea.fe3, 16);
}

void printMem(Context &ctx)
{
    cout << "Memory:" << endl;
    uint64_t i = 0;
    for (map<uint64_t, Fea>::iterator it = ctx.mem.begin(); it != ctx.mem.end(); it++)
    {
        mpz_class addr(it->first);
        cout << "i: " << i << " address:" << addr.get_str(16) << " ";
        cout << printFea(ctx, it->second);
        cout << endl;
        i++;
    }
}

#ifdef USE_LOCAL_STORAGE
void printStorage(Context &ctx)
{
    uint64_t i = 0;
    for (map<Goldilocks::Element, mpz_class, CompareFe>::iterator it = ctx.sto.begin(); it != ctx.sto.end(); it++)
    {
        Goldilocks::Element fe = it->first;
        mpz_class scalar = it->second;
        cout << "Storage: " << i << " fe: " << ctx.fr.toString(fe, 16) << " scalar: " << scalar.get_str(16) << endl;
    }
}
#endif

void printReg(Context &ctx, string name, Goldilocks::Element &fe, bool h, bool bShort)
{
    cout << "    Register: " << name << " Value: " << ctx.fr.toString(fe, 16) << endl;
}

void printDb(Context &ctx)
{
    ctx.db.print();
}

void printU64(Context &ctx, string name, uint64_t v)
{
    cout << "    U64: " << name << ":" << v << endl;
}

void printU32(Context &ctx, string name, uint32_t v)
{
    cout << "    U32: " << name << ":" << v << endl;
}

void printU16(Context &ctx, string name, uint16_t v)
{
    cout << "    U16: " << name << ":" << v << endl;
}

void printBa(uint8_t * pData, uint64_t dataSize, string name)
{
    cout << name << " = ";
    for (uint64_t k=0; k<dataSize; k++)
    {
        cout << byte2string(pData[k]) << ":";
    }
    cout << endl;
}

void printBits(uint8_t * pData, uint64_t dataSize, string name)
{
    cout << name << " = ";
    for (uint64_t k=0; k<dataSize/8; k++)
    {
        uint8_t byte;
        bits2byte(pData+k*8, byte);
        cout << byte2string(byte) << ":";
    }
    cout << endl;
}

string rt2string(eReferenceType rt)
{
    switch (rt)
    {
    case rt_unknown:
        return "rt_unknown";
    case rt_pol:
        return "rt_pol";
    case rt_field:
        return "rt_field";
    case rt_treeGroup:
        return "rt_treeGroup";
    case rt_treeGroup_elementProof:
        return "rt_treeGroup_elementProof";
    case rt_treeGroup_groupProof:
        return "rt_treeGroup_groupProof";
    case rt_treeGroupMultipol:
        return "rt_treeGroupMultipol";
    case rt_treeGroupMultipol_groupProof:
        return "rt_treeGroupMultipol_groupProof";
    case rt_idxArray:
        return "rt_idxArray";
    case rt_int:
        return "rt_int";
    default:
        cerr << "rt2string() found unrecognized reference type: " << rt << endl;
        exit(-1);
    }
    enum eReferenceType
    {
        rt_unknown = 0,
        rt_pol = 1,
        rt_field = 2,
        rt_treeGroup = 3,
        rt_treeGroup_elementProof = 4,
        rt_treeGroup_groupProof = 5,
        rt_treeGroupMultipol = 6,
        rt_treeGroupMultipol_groupProof = 7,
        rt_idxArray = 8,
        rt_int = 9
    };
}
/*
string calculateExecutionHash(Goldilocks &fr, Reference &ref, string prevHash)
{
    switch (ref.type)
    {
    case rt_pol:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        std::string currentHash;
        std::string newHash;
        string pol;
        for (uint64_t i = 0; i < ref.N; i++)
        {
            pol += fr.toString(ref.pPol[i], 16);
        }
        MD5((unsigned char *)pol.c_str(), pol.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    case rt_field:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        std::string currentHash;
        std::string newHash;
        string pol = fr.toString(ref.fe, 16);

        MD5((unsigned char *)pol.c_str(), pol.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    case rt_treeGroup:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        string currentHash;
        string newHash;
        string auxHash;
        string tempConcatHashes;
        string pol;
        Merkle M(MERKLE_ARITY);
        uint32_t groupSize = M.numHashes(ref.groupSize);
        uint64_t k = 0;
        for (; k < (ref.memSize / sizeof(Goldilocks::Element)) - groupSize * ref.nGroups; k++)
        {
            pol += fr.toString(ref.pTreeGroup[k], 16);
        }
        MD5((unsigned char *)pol.c_str(), pol.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            auxHash.append(tempHash);
        }
        tempConcatHashes.append(auxHash);
        pol = "";
        auxHash = "";
        for (; k < (ref.memSize / sizeof(Goldilocks::Element)); k++)
        {
            pol += fr.toString(ref.pTreeGroup[k], 16);
        }
        MD5((unsigned char *)pol.c_str(), pol.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            auxHash.append(tempHash);
        }

        tempConcatHashes.append(auxHash);
        MD5((unsigned char *)tempConcatHashes.c_str(), tempConcatHashes.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }

    case rt_treeGroup_elementProof:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        std::string currentHash;
        std::string newHash;
        string pol;
        for (uint64_t i = 0; i < ref.memSize / sizeof(Goldilocks::Element); i++)
        {
            pol += fr.toString(ref.pTreeGroup_elementProof[i], 16);
        }
        MD5((unsigned char *)pol.c_str(), pol.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    case rt_treeGroup_groupProof:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        std::string currentHash;
        std::string newHash;
        string pol;
        for (uint64_t i = 0; i < ref.memSize / sizeof(Goldilocks::Element); i++)
        {
            pol += fr.toString(ref.pTreeGroup_groupProof[i], 16);
        }
        MD5((unsigned char *)pol.c_str(), pol.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    case rt_treeGroupMultipol:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        std::string currentHash;
        std::string newHash;
        string pol;
        string auxHash;
        string tempConcatHashes;
        Merkle M(MERKLE_ARITY);
        uint32_t polProofSize = M.numHashes(ref.nPols);

        for (uint32_t k = 0; k < (ref.memSize / sizeof(Goldilocks::Element)); k++)
        {
            if (k % (polProofSize + ref.groupSize) == 0 && k <= (polProofSize + ref.groupSize) * ref.nGroups && k != 0)
            {
                MD5((unsigned char *)pol.c_str(), pol.size(), result);
                for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
                {
                    sprintf(tempHash, "%02x", result[i]);
                    auxHash.append(tempHash);
                }
                tempConcatHashes.append(auxHash);
                pol = "";
                auxHash = "";
            }
            pol.append(fr.toString(ref.pTreeGroupMultipol[k], 16));
        }
        MD5((unsigned char *)pol.c_str(), pol.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            auxHash.append(tempHash);
        }
        tempConcatHashes.append(auxHash);
        MD5((unsigned char *)tempConcatHashes.c_str(), tempConcatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }

        string concatHashes = prevHash.append(currentHash);
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    case rt_treeGroupMultipol_groupProof:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        std::string currentHash;
        std::string newHash;
        string pol;
        for (uint64_t i = 0; i < ref.memSize / sizeof(Goldilocks::Element); i++)
        {
            pol += fr.toString(ref.pTreeGroupMultipol_groupProof[i], 16);
        }
        MD5((unsigned char *)pol.c_str(), pol.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    case rt_idxArray:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];

        string array;
        for (uint32_t k = 0; k < ref.N; k++)
        {
            array.append(std::to_string(ref.pIdxArray[k]));
        }
        MD5((unsigned char *)array.c_str(), array.size(), result);
        std::string currentHash;
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        std::string newHash;
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    case rt_int:
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        char tempHash[32];
        std::string currentHash;
        std::string newHash;
        string array = std::to_string(ref.integer);

        MD5((unsigned char *)array.c_str(), array.size(), result);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            currentHash.append(tempHash);
        }
        string concatHashes = prevHash + currentHash;
        MD5((unsigned char *)concatHashes.c_str(), concatHashes.size(), result);

        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(tempHash, "%02x", result[i]);
            newHash.append(tempHash);
        }
        return newHash;
    }
    default:
    {
        cerr << "  printReference() found unrecognized reference type: " << ref.type << endl;
        exit(-1);
    }
    }
}
*/

void printReference(Goldilocks &fr, Reference &ref)
{
    cout << "  Reference of type: " << rt2string(ref.type) << endl;
    switch (ref.type)
    {
    case rt_pol:
    {
        cout << "  ref.N: " << ref.N << endl;
        cout << "  ref.elementType: " << et2string(ref.elementType) << endl;
        cout << "  ref.memSize: " << ref.memSize << endl;
        uint64_t printed = 0;
        for (uint64_t i = 0; i < ref.N; i++)
        {
            if (fr.isZero(ref.pPol[i]))
                continue;
            if (i > 5 && i < ref.N - 5)
                continue;
            if (printed < 10)
                cout << "  ref.pPol[" << i << "]: " << fr.toString(ref.pPol[i], 16) << endl;
            printed++;
        }
        cout << "  found " << printed << " non-zero elements" << endl;
        return;
    }
    case rt_field:
    {
        cout << "  ref.fe: " << fr.toString(ref.fe, 16) << endl;
        return;
    }
    case rt_treeGroup:
    {
        cout << "  ref.elementType: " << ref.elementType << endl;
        cout << "  ref.memSize: " << ref.memSize << endl;
        cout << "  ref.memSize: " << ref.nGroups << endl;
        cout << "  ref.memSize: " << ref.groupSize << endl;
        cout << "  ref.memSize: " << ref.nPols << endl;

        cout << "  ref.pTreeGroupMultipol[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroup[0], 16) << endl;
        cout << "  ref.pTreeGroupMultipol[" << 0 << "][" << 1 << "]: " << fr.toString(ref.pTreeGroup[1], 16) << endl;
        cout << "  ref.pTreeGroupMultipol[last - 1]: " << fr.toString(ref.pTreeGroup[(ref.memSize / sizeof(Goldilocks::Element)) - 2], 16) << endl;
        cout << "  ref.pTreeGroupMultipollast]: " << fr.toString(ref.pTreeGroup[(ref.memSize / sizeof(Goldilocks::Element)) - 1], 16) << endl;
        return;
    }

    case rt_treeGroup_elementProof:
    {
        cout << "  ref.elementType: " << ref.elementType << endl;
        cout << "  ref.memSize: " << ref.memSize << endl;
        cout << "  ref.memSize: " << ref.nGroups << endl;
        cout << "  ref.memSize: " << ref.groupSize << endl;

        cout << "  ref.pTreeGroup_elementProof[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroup_elementProof[0], 16) << endl;
        cout << "  ref.pTreeGroup_elementProof[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroup_elementProof[1], 16) << endl;
        cout << "  ref.pTreeGroup_elementProof[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroup_elementProof[(ref.memSize / sizeof(Goldilocks::Element)) - 2], 16) << endl;
        cout << "  ref.pTreeGroup_elementProof[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroup_elementProof[(ref.memSize / sizeof(Goldilocks::Element)) - 1], 16) << endl;

        return;
    }
    case rt_treeGroup_groupProof:
    {
        cout << "  ref.elementType: " << ref.elementType << endl;
        cout << "  ref.memSize: " << ref.memSize << endl;
        cout << "  ref.nGroups: " << ref.nGroups << endl;
        cout << "  ref.groupSize: " << ref.groupSize << endl;

        cout << "  ref.pTreeGroup_groupProof[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroup_groupProof[0], 16) << endl;
        cout << "  ref.pTreeGroup_groupProof[" << 0 << "][" << 1 << "]: " << fr.toString(ref.pTreeGroup_groupProof[1], 16) << endl;
        cout << "  ref.pTreeGroup_groupProof[0]: " << fr.toString(ref.pTreeGroup_groupProof[(ref.memSize / sizeof(Goldilocks::Element)) - 2], 16) << endl;
        cout << "  ref.pTreeGroup_groupProof[last]: " << fr.toString(ref.pTreeGroup_groupProof[(ref.memSize / sizeof(Goldilocks::Element)) - 1], 16) << endl;
        return;
    }
    case rt_treeGroupMultipol:
    {
        cout << "  ref.elementType: " << ref.elementType << endl;
        cout << "  ref.memSize: " << ref.memSize << endl;
        cout << "  ref.memSize: " << ref.nGroups << endl;
        cout << "  ref.memSize: " << ref.groupSize << endl;
        cout << "  ref.memSize: " << ref.nPols << endl;

        cout << "  ref.pTreeGroupMultipol[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroupMultipol[0], 16) << endl;
        cout << "  ref.pTreeGroupMultipol[" << 0 << "][" << 1 << "]: " << fr.toString(ref.pTreeGroupMultipol[1], 16) << endl;
        cout << "  ref.pTreeGroupMultipolMainTree[0]: " << fr.toString(ref.pTreeGroupMultipol[(ref.memSize / sizeof(Goldilocks::Element)) - 2], 16) << endl;
        cout << "  ref.pTreeGroupMultipolMainTree[last]: " << fr.toString(ref.pTreeGroupMultipol[(ref.memSize / sizeof(Goldilocks::Element)) - 1], 16) << endl;
        return;
    }
    case rt_treeGroupMultipol_groupProof:
    {
        cout << "  ref.elementType: " << ref.elementType << endl;
        cout << "  ref.memSize: " << ref.memSize << endl;
        cout << "  ref.memSize: " << ref.nGroups << endl;
        cout << "  ref.memSize: " << ref.groupSize << endl;
        cout << "  ref.memSize: " << ref.nPols << endl;

        cout << "  ref.pTreeGroupMultipol[" << 0 << "][" << 0 << "]: " << fr.toString(ref.pTreeGroupMultipol_groupProof[0], 16) << endl;
        cout << "  ref.pTreeGroupMultipol[" << 0 << "][" << 1 << "]: " << fr.toString(ref.pTreeGroupMultipol_groupProof[1], 16) << endl;
        cout << "  ref.pTreeGroupMultipolMainTree[0]: " << fr.toString(ref.pTreeGroupMultipol_groupProof[(ref.memSize / sizeof(Goldilocks::Element)) - 2], 16) << endl;
        cout << "  ref.pTreeGroupMultipolMainTree[last]: " << fr.toString(ref.pTreeGroupMultipol_groupProof[(ref.memSize / sizeof(Goldilocks::Element)) - 1], 16) << endl;
        return;
    }
    case rt_idxArray:
    {
        cout << "  ref.N: " << ref.N << endl;
        cout << "  ref.elementType: " << et2string(ref.elementType) << endl;
        cout << "  ref.memSize: " << ref.memSize << endl;
        uint64_t printed = 0;
        for (uint64_t i = 0; i < ref.N; i++)
        {
            if (ref.pIdxArray[i] == 0)
                continue;
            if (printed < 10)
                cout << "  ref.pIdxArray[" << i << "]: " << ref.pIdxArray[i] << endl;
            printed++;
        }
        cout << "  found " << printed << " non-zero elements" << endl;
        return;
    }
    case rt_int:
    {
        cout << "  ref.integer: " << ref.integer << endl;
        return;
    }
    default:
    {
        cerr << "  printReference() found unrecognized reference type: " << ref.type << endl;
        exit(-1);
    }
    }
}

string getTimestamp (void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    char tmbuf[64], buf[256];
    strftime(tmbuf, sizeof tmbuf, "%Y%m%d_%H%M%S", gmtime(&tv.tv_sec));
    snprintf(buf, sizeof buf, "%s_%03ld", tmbuf, tv.tv_usec/1000);
    return buf;
}

string getUUID (void)
{
    char uuidString[37];
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, uuidString);
    return uuidString;
}

void json2file(const json &j, const string &fileName)
{
    ofstream outputStream(fileName);
    if (!outputStream.good())
    {
        cerr << "Error: json2file() failed loading output JSON file " << fileName << endl;
        exit(-1);
    }
    outputStream << setw(4) << j << endl;
    outputStream.close();
}

void file2json(const string &fileName, json &j)
{
    std::ifstream inputStream(fileName);
    if (!inputStream.good())
    {
        cerr << "Error: file2json() failed loading input JSON file " << fileName << endl;
        exit(-1);
    }
    inputStream >> j;
    inputStream.close();
}

void * mapFile (const string &fileName, uint64_t size, bool bOutput)
{
    // If input, check the file size is the same as the expected polsSize
    if (!bOutput)
    {
        struct stat sb;
        if ( lstat(fileName.c_str(), &sb) == -1)
        {
            cerr << "Error: Pols::mapToFile() failed calling lstat() of file " << fileName << endl;
            exit(-1);
        }
        if ((uint64_t)sb.st_size != size)
        {
            cerr << "Error: Pols::mapToFile() found size of file " << fileName << " to be " << sb.st_size << " B instead of " << size << " B" << endl;
            exit(-1);
        }
    }

    // Open the file withe the proper flags
    int oflags;
    if (bOutput) oflags = O_CREAT|O_RDWR|O_TRUNC;
    else         oflags = O_RDWR;
    int fd = open(fileName.c_str(), oflags, 0666);
    if (fd < 0)
    {
        cerr << "Error: mapFile() failed opening file: " << fileName << endl;
        exit(-1);
    }

    // If output, extend the file size to the required one
    if (bOutput)
    {
        // Seek the last byte of the file
        int result = lseek(fd, size-1, SEEK_SET);
        if (result == -1)
        {
            cerr << "Error: mapFile() failed calling lseek() of file: " << fileName << endl;
            exit(-1);
        }

        // Write a 0 at the last byte of the file, to set its size; content is all zeros
        result = write(fd, "", 1);
        if (result < 0)
        {
            cerr << "Error: mapFile() failed calling write() of file: " << fileName << endl;
            exit(-1);
        }
    }

    // Map the file into memory
    void * pAddress;
    pAddress = (uint8_t *)mmap( NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (pAddress == MAP_FAILED)
    {
        cerr << "Error: mapFile() failed calling mmap() of file: " << fileName << endl;
        exit(-1);
    }
    close(fd);

    return pAddress;
}

void unmapFile (void * pAddress, uint64_t size)
{
    int err = munmap(pAddress, size);
    if (err != 0)
    {
        cerr << "Error: unmapFile() failed calling munmap() of address=" << pAddress << " size=" << size << endl;
        exit(-1);
    }
}