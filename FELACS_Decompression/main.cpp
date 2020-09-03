#include <fstream>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <math.h>       /* round, floor, ceil, trunc */
#include <tgmath.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <numeric>
#include "dirent.h"

using namespace std;
/* Decompress Function */
string* get_Filenames(char* filePath, int totalFiles);
int get_TotalFiles(char* filePath);
std::vector<uint16_t> FELACS_Decompress(std::vector<uint8_t> Data_Compressed, int blockSize, int totalSamples, int dataColumns);

int main()
{
    char originalFilePath[] = "C:/Users/User/OneDrive/THESIS/2_Compression_Algorithms/C++(git)/Data_Input_Compression";
    char compressedFilePath[] = "C:/Users/User/OneDrive/THESIS/2_Compression_Algorithms/C++(git)/Data_Compressed";
    int totalFilesCompressed = get_TotalFiles(compressedFilePath);
    int totalFilesOriginal = get_TotalFiles(originalFilePath);
    if(totalFilesCompressed == totalFilesOriginal)
    {
        cout << "all original files are compressed" << endl;
    }
    else
    {
        cout << "not all files are compressed, perform this first!!" << endl;
        exit(1);
    }
    /* Variables for compression */
    int blockSize = 265;
    int dataColumns = 48;
    /* Variables for creating output files */
    ostringstream os_filePath;
    ostringstream os_fileName;
    string* fileNamesCompressed;
    string* fileNamesOriginal;
    string inputFileNameCompressed = "";
    string inputPathCompressed = "";
    string inputFileNameOriginal = "";
    string inputPathOriginal = "";
    string outputFileName = "";
    string outputPath = "";
    /* Variables for reading files */
    uint8_t readUint8;
    uint16_t readUint16;
    int16_t readInt16;

    fileNamesCompressed = get_Filenames(compressedFilePath, totalFilesCompressed);
    fileNamesOriginal = get_Filenames(originalFilePath, totalFilesOriginal);
    /* There seems to be an offset of 2 in the read files */
    for(int i = 2 ; i < totalFilesCompressed ; ++i)
    {
        /* Create vectors to store all data types in */
        std::vector<uint8_t> Data_Compressed;
        std::vector<uint16_t> Data_Decompressed;
        std::vector<uint16_t> Data_Original_Uint16;
        std::vector<int16_t> Data_Decompressed_Int16;
        /* Set variables for reading compressed files */
        inputFileNameCompressed = "";
        inputPathCompressed = "";
        os_filePath.str("");
        os_fileName.str("");
        os_filePath.clear();
        os_fileName.clear();
        os_filePath << compressedFilePath << "/" << fileNamesCompressed[i];
        inputPathCompressed = os_filePath.str();
        os_fileName << fileNamesCompressed[i];
        inputFileNameCompressed = os_fileName.str();
        /* Set variables for reading original files */
        inputFileNameOriginal = "";
        inputPathOriginal = "";
        os_filePath.str("");
        os_fileName.str("");
        os_fileName.clear();
        os_filePath.clear();
        os_filePath << originalFilePath << "/" << fileNamesOriginal[i];
        inputPathOriginal = os_filePath.str();
        os_fileName << fileNamesOriginal[i];
        inputFileNameOriginal = os_fileName.str();

        std::ifstream inputCompressedFile(inputPathCompressed, std::ios::in | std::ifstream::binary);
        std::ifstream inputOriginalFile(inputPathOriginal, std::ios::in | std::ifstream::binary);
        /* Check if files can be opened */
        if (!inputCompressedFile.is_open())
        {
            std::cerr << "There was a problem opening the" << inputFileNameCompressed << "file for compression!" << endl;;
            exit(1);//exit or do additional error checking
        }
        /* Check if files can be opened */
        if (!inputOriginalFile.is_open())
        {
            std::cerr << "There was a problem opening the" << inputFileNameOriginal << "file for compression!" << endl;;
            exit(1);//exit or do additional error checking
        }
        /* Read original file */
        while (!inputCompressedFile.eof())
        {
            readUint8 = inputCompressedFile.get();
            Data_Compressed.push_back(readUint8);
        }
        Data_Compressed.pop_back();
        if (inputFileNameOriginal.find("int16") != std::string::npos || inputFileNameOriginal.find("Int16") != std::string::npos)
        {
            while (!inputOriginalFile.eof())
            {
                readInt16 = 0;
                readInt16 |= (unsigned char)inputOriginalFile.get();
                readInt16 |= inputOriginalFile.get() << 8;
                Data_Original_Uint16.push_back(uint16_t(int32_t(readInt16) + int32_t(pow(2,15))));
            }
        }
        else
        {
            while (!inputOriginalFile.eof())
            {
                readUint16 = 0;
                readUint16 |= (unsigned char)inputOriginalFile.get();
                readUint16 |= inputOriginalFile.get() << 8;
                Data_Original_Uint16.push_back(readUint16);
            }
        }
        Data_Original_Uint16.pop_back();
        /* display filename being decompressed */
        std::cout << "File being decompressed: " << inputFileNameCompressed << endl;
        /* Decompress file */
        Data_Decompressed = FELACS_Decompress(Data_Compressed, blockSize, Data_Original_Uint16.size(), dataColumns);
        /* Check if data is the same */
        if(Data_Decompressed.size() == Data_Original_Uint16.size())
        {
            cout << "size compressed and decompressed same!" << endl;
        }
        else
        {
            cout << "WARNING: DIFFERENCE IN SIZE: " << (Data_Original_Uint16.size() - Data_Decompressed.size()) << endl;
        }
        for(uint32_t i = 0; i < Data_Decompressed.size() ; ++i)
        {
            if(Data_Decompressed.at(i) != Data_Original_Uint16.at(i))
            {
                cout << "ERROR DECOMPRESSION FAILED " << i << " :     "  << Data_Decompressed.at(i) << "    should be   " << Data_Original_Uint16.at(i) << endl;
            }
        }

        /* Write decompressed file to txt file */
        outputPath = "";
        os_filePath.str("");
        os_filePath.clear();
        os_filePath << "C:/Users/User/OneDrive/THESIS/2_Compression_Algorithms/C++(git)/Data_Decompressed/Decompressed_" << fileNamesOriginal[i];
        outputPath =  os_filePath.str();
        cout << outputPath << endl;
        std::ofstream outputFile(outputPath, std::ios::out | std::ofstream::binary);
        /* If the file is Uint16 the algorithm will think it is a int16 file, FIX THIS!! */
        if (inputFileNameOriginal.find("int16") != std::string::npos || inputFileNameOriginal.find("Int16") != std::string::npos)
        {
            for(uint64_t i = 0 ; i < Data_Decompressed.size() ; ++i)
            {
                Data_Decompressed_Int16.push_back(int16_t(int32_t(Data_Decompressed[i]) - int32_t(pow(2,15))));
                outputFile.write(reinterpret_cast<const char *>(&Data_Decompressed_Int16[i]), sizeof(Data_Decompressed_Int16[i]));
            }
        }
        else
        {
            for(uint64_t i = 0 ; i < Data_Decompressed.size() ; ++i)
            {
                outputFile.write(reinterpret_cast<const char *>(&Data_Decompressed[i]), sizeof(Data_Decompressed[i]));
            }
        }
        cout << "decompression succes!!" << endl;

    }

//    /* Vectors for storing decompressed data */
//    std::vector<uint16_t> Temp_LU_84_Decompressed;
//    std::vector<uint16_t> acc_x_11_Decompressed;
//    /* Load input files for decompression */
//    std::ifstream Temp_LU_84_Compressed_ifile("C:/Users/basti/OneDrive/THESIS/2_Compression_Algorithms/Algorithms/C++/CodeBlocks/FELACS_Compression/Temp_LU_84_Compressed.txt", std::ios::in | std::ios::binary);
//    std::ifstream acc_x_11_Compressed_ifile("C:/Users/basti/OneDrive/THESIS/2_Compression_Algorithms/Algorithms/C++/CodeBlocks/FELACS_Compression/acc_x_11_Compressed.txt", std::ios::in | std::ios::binary);
//    /* Create files for storing the results of decompression */
//    std::ofstream Temp_LU_84_Deompressed_ofile("Temp_LU_84_Decompressed.txt", std::ios::out | std::ofstream::binary);
//    std::ofstream acc_x_11_Deompressed_ofile("acc_x_11_Decompressed.txt", std::ios::out | std::ofstream::binary);
//    /* Check if file is opened correctly */
//    if (!Temp_LU_84_Compressed_ifile.is_open())
//    {
//        std::cerr << "There was a problem opening the Temp_LU_84_Compressed_ifile file!\n";
//        exit(1);//exit or do additional error checking
//    }
//    if (!acc_x_11_Compressed_ifile.is_open())
//    {
//        std::cerr << "There was a problem opening the acc_x_11_Compressed_ifile file!\n";
//        exit(1);//exit or do additional error checking
//    }
//
//    /* Store compressed data into a uint8_t vector */
//    std::vector<uint8_t> Temp_LU_84_Compressed((std::istreambuf_iterator<char>(Temp_LU_84_Compressed_ifile)), std::istreambuf_iterator<char>());
//    cout << "Size: " << Temp_LU_84_Compressed.size() << endl;
//    std::vector<uint8_t> acc_x_11_Compressed((std::istreambuf_iterator<char>(acc_x_11_Compressed_ifile)), std::istreambuf_iterator<char>());
//    cout << "Size: " << acc_x_11_Compressed.size() << endl;
//
//    /* Decompress file */
//    Temp_LU_84_Decompressed = FELACS_Decompress(Temp_LU_84_Compressed, 264, 64913);
//    acc_x_11_Decompressed = FELACS_Decompress(acc_x_11_Compressed, 264, 63984);
//
//    /* Write decompressed data into .txt file */
//    for(uint64_t i = 0 ; i < Temp_LU_84_Decompressed.size() ; ++i)
//    {
//        Temp_LU_84_Deompressed_ofile.write(reinterpret_cast<const char *>(&Temp_LU_84_Decompressed[i]), sizeof(Temp_LU_84_Decompressed[i]));
//    }
//    Temp_LU_84_Deompressed_ofile.close();
//    for(uint64_t i = 0 ; i < acc_x_11_Decompressed.size() ; ++i)
//    {
//        acc_x_11_Deompressed_ofile.write(reinterpret_cast<const char *>(&acc_x_11_Decompressed[i]), sizeof(acc_x_11_Decompressed[i]));
//    }
//    acc_x_11_Deompressed_ofile.close();
    return 0;
}

string* get_Filenames(char* filePath, int totalFiles)
{
    DIR *dir;
    string* matrix = new string[totalFiles];
    struct dirent *ent;
    /* read files from directory */
    if ((dir = opendir (filePath)) != NULL)
    {
        /* print all the files and directories within directory */
        for(int i = 0 ; (ent = readdir (dir)) != NULL ; ++i)
        {
            matrix[i] = ent->d_name;
//            cout << matrix[i] << endl;
        }
        closedir (dir);
    }
    else
    {
        /* could not open directory */
        perror ("Failed to get files!");
    }
    return matrix;
}

int get_TotalFiles(char* filePath)
{
    int totalFiles = 0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (filePath)) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL)
        {
            ++totalFiles;
        }
        closedir (dir);
    }
    else
    {
        /* could not open directory */
        perror ("Failed to get files!");
    }
    return totalFiles;
}

std::vector<uint16_t> FELACS_Decompress(std::vector<uint8_t> Data_Compressed, int blockSize, int totalSamples, int dataColumns)
{
    cout << "begin decompress" << endl;
    std::vector<uint16_t> Data_Decompressed;
    std::vector<uint16_t> Data_Decompressed_temp;
    Data_Decompressed.reserve(totalSamples);
    Data_Decompressed_temp.reserve(totalSamples);
    uint16_t firstSample;
    uint16_t k_optimal;
    uint64_t filledBits;
    int si;
    uint16_t ai;
    int32_t omega;
    int32_t temp;
    int32_t increase;
    int N = 16;
    uint16_t lastElement;
    int8_t lengthai;

    int offset = 0;

    for(int i = 0 ; i < (totalSamples/(blockSize*dataColumns)) ; ++i)
    {
        for(int l = 0 ; l < dataColumns; l++)
        {
            filledBits = 0;
            firstSample = (Data_Compressed.at(offset + 1) << 8) + Data_Compressed.at(offset + 0);
            Data_Decompressed.push_back(firstSample);
            k_optimal = (Data_Compressed.at(offset + 2) & 0x07);
            //cout << "k" << k_optimal << " | " << firstSample << endl;
            filledBits += 3 + 8*2;
            for(int j = 0 ; j < blockSize-1 ; ++j)
            {
                si = 0;
                ai = 0;
                while((Data_Compressed.at(offset + (filledBits / 8)) & (1 << (filledBits % 8))) == 0 )
                {
                    si += 1;
                    filledBits += 1;
                }
                // zero detected hence si is increased one and filledBits to
                si += 1;
                filledBits += 1;
                if(si >= 2 )
                {
                    lengthai = k_optimal+si-2;
                }
                else
                {
                    lengthai = k_optimal;
                }
                for(int k = 0 ; k < lengthai ; ++k)
                {
                    int tempOffset = filledBits / 8;
                    // check if a 0 or 1 is in the j'bit of Compressed
                    if((Data_Compressed.at(offset + tempOffset) & (1 << (filledBits % 8))) != 0 )
                    {
                        ai += (1 << k);
                    }
                    filledBits += 1;
                }
                // reverse engineer
                if(si == 1)
                {
                    temp = ai;
                }
                else
                {
                    temp = pow(2,k_optimal + si - 2) + ai;
                }


                // inversed function
                omega = int32_t(std::min(int32_t(Data_Decompressed.at(j + l*blockSize + i*blockSize*dataColumns)), int32_t(pow(2,N) - 1 - Data_Decompressed.at(j + l*blockSize + i*blockSize*dataColumns))));
                lastElement = Data_Decompressed.at(j + l*blockSize + i*blockSize*dataColumns);
                //if(k_optimal == 7 && si == 10) cout << "omega: " << omega << " | lengthai: " << float(lengthai)  << " | temp: " << temp << " | lastelement: " << lastElement  << endl;
                if(temp <= 2*omega)
                {
                    if((temp % 2) == 1)
                    {
                        increase = -int32_t((temp + 1)/2);
                    }
                    else
                    {
                        increase = int32_t(temp/2);
                    }
                }
                else
                {
                    if(lastElement < uint16_t(pow(2,(N-1))))
                    {
                        increase = temp-omega;
                    }
                    else
                    {
                        increase = -(temp-omega);
                    }
                }
                Data_Decompressed.push_back(uint16_t(int32_t(lastElement) + increase)); //insert delta on top of offset
//                if(j + i*blockSize > 28300 && j + i*blockSize < 28500)
//                cout << (j + i*blockSize) <<  "|    k" << k_optimal << " | " << firstSample << " | " << "si: " << si << " | " << "ai: " << ai << "  |  " << "lastelement: " << lastElement << "   |   "  << "increase: " << increase << "   |   " << "omega: " << omega <<  endl;
            }
            if((filledBits % 8) != 0)
            {
                offset += (filledBits / 8) + 1;
            }
            else
            {
                offset += (filledBits / 8);
            }
        }
    }
    for (int32_t i=0; i<int32_t(Data_Decompressed.size()); i++)
    {
        Data_Decompressed_temp.push_back(Data_Decompressed[i]);
//        cout << i << "|     " << Data_Decompressed[i] << endl;
    }

    /* Transpose the blocks */
    for(int i = 0; i < (totalSamples/(blockSize*dataColumns)) ; ++i)
    {
        for(int j = 0; j < blockSize; ++j)
        {
            for(int k = 0; k < dataColumns; ++k)
            {
//                cout << "before: " << Data_Decompressed.at(k + j*blockSize + i*blockSize*dataColumns) << endl;
                Data_Decompressed.at(k + j*dataColumns + i*blockSize*dataColumns) = Data_Decompressed_temp.at(k*blockSize + j + i*blockSize*dataColumns);
//               cout << "after: " << Data_Decompressed.at(k + j*blockSize + i*blockSize*dataColumns) << "| "<< (k*blockSize + j +  i*blockSize*dataColumns) <<  endl;
            }
        }
    }
    return Data_Decompressed;
}
