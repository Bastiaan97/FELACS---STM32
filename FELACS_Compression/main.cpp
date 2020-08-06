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
/* Function delcerations */
std::vector<uint16_t> temperatureConversion(std::vector<double> Temp_F);
std::vector<uint64_t> pdf(std::vector<uint16_t> Data, int byteRange);
std::vector<uint8_t> FELACS(std::vector<uint16_t> Data, int blocksize, int N, int dataColumns);
double entropy(std::vector<uint64_t> Data);
int get_TotalFiles(char* filePath);
string* get_Filenames(char* filePath, int totalFiles);


int main()
{
    /* Variables for reading files */
    double readDouble;
    int16_t readInt16;
    uint16_t readUint16;

    /* Variables for creating output files */
    ostringstream os_filePath;
    ostringstream os_fileName;
    string* fileNames;
    string inputFileName = "";
    string inputPath = "";
    string outputFileName = "";
    string outputPath = "";
    std::vector<double> CRforPrinting;
    /* Read files from filepath */
    char filePath[] = "C:/Users/User/OneDrive/THESIS/2_Compression_Algorithms/C++(Git)/Data_Input_Compression";
    int totalFiles = get_TotalFiles(filePath);
    /* Variables for compression */
    int blockSize = 265;
    int dataColumns = 48;


    fileNames = get_Filenames(filePath, totalFiles);
    /* There seems to be an offset of 2 in the read files */
    for(int i = 2 ; i < totalFiles ; ++i)
    {
        /* Create vectors to store all data types in */
        inputFileName = "";
        inputPath = "";
        outputFileName = "";
        outputPath = "";
        std::vector<uint16_t> Data_uint16;
        std::vector<uint16_t> Data_uint16_temp;
        std::vector<int16_t> Data_int16;
        std::vector<double> Data_double;
        std::vector<uint64_t> Data_pdf;
        std::vector<uint8_t> FELACS_Compressed;
        int offset = 0;
        double Data_entropy;
        double Data_CR;
        os_filePath.str("");
        os_filePath.clear();
        os_fileName.str("");
        os_fileName.clear();
        outputPath = "";
        os_filePath << "C:/Users/User/OneDrive/THESIS/2_Compression_Algorithms/C++(Git)/Data_Compressed/Compressed_" << fileNames[i];
        outputPath =  os_filePath.str();
        os_fileName << fileNames[i] << "_ofile";
        cout << "created: Compressed_" << fileNames[i] << " | in: " << outputPath << endl;
        std::ofstream outputFile(outputPath, std::ios::out | std::ofstream::binary);
        os_filePath.str("");
        os_filePath.clear();
        os_fileName.str("");
        os_fileName.clear();
        outputPath = "";
        os_filePath << filePath << "/" << fileNames[i];
        inputPath = os_filePath.str();
        os_fileName << fileNames[i];
        inputFileName = os_fileName.str();
        std::ifstream inputFile(inputPath, std::ios::in | std::ifstream::binary);
        /* Check if files can be opened */
        if (!inputFile.is_open())
        {
            std::cerr << "There was a problem opening the" << inputFileName << "file!" << endl;;
            exit(1);//exit or do additional error checking
        }

        /* Read files and store data according to the format of the file */

        if (inputFileName.find("int16") != std::string::npos || inputFileName.find("Int16") != std::string::npos)
        {
            int tempCount = 0;
            /* read data format int16, store this value and convert to data range of uint16 */
            while (!inputFile.eof())
            {
                readInt16 = 0;
                readInt16 |= (unsigned char)inputFile.get();
                readInt16 |= inputFile.get() << 8;
                Data_int16.push_back(readInt16);

                Data_uint16.push_back(uint16_t(int32_t(readInt16) + int32_t(pow(2,15))));
            }
            Data_int16.pop_back();  // delete last double occurring element
            Data_uint16.pop_back(); // delete last double occurring element
            inputFile.close();
            /* display filename being compressed */
            std::cout << "File name: " << inputFileName << endl;
            /* calculate probability distribution function */
            Data_pdf = pdf(Data_uint16, 16);
            /* calculate and display source entropy */
            Data_entropy = entropy(Data_pdf);
            cout << "Entropy: " << Data_entropy << endl;
            /* compress data file using FELACS algorithm */
            /* save parts of size 265 x dataColumns and compress it */
            for(int j = 0; j < Data_int16.size()/(dataColumns * blockSize); ++j)
            {
                offset = dataColumns * blockSize;
                for(int k = 0; k < (dataColumns * blockSize); k++)
                {
                    Data_uint16_temp.push_back(Data_int16.at(k + j*offset));
                }
                FELACS_Compressed = FELACS(Data_uint16_temp, blockSize, 16, dataColumns);
                /* Calculate compression ratio */
                Data_CR = (1 - (double(FELACS_Compressed.size())) / (double(Data_uint16_temp.size()*2)))*100;
                cout << "Compression Ration: " << Data_CR << "%" << endl;
                cout << "bits per sample: " << (16 * ((100 - Data_CR)/100)) << endl;
                /* Calculate adjusted compression ratio */
                Data_CR = (1 - (double(FELACS_Compressed.size())) / (double(Data_uint16_temp.size()*2 - (Data_uint16_temp.size() % blockSize)*2)))*100;
                cout << "Correct compression ratio: " << Data_CR << "%" << endl;
                CRforPrinting.push_back(Data_CR);
                cout << "bits per sample: " << (16 * ((100 - Data_CR)/100)) << endl;
                // step 5
                for(uint64_t i = 0 ; i < FELACS_Compressed.size() ; ++i)
                {
                    outputFile.write(reinterpret_cast<const char *>(&FELACS_Compressed[i]), sizeof(FELACS_Compressed[i]));
                }
                while (!Data_uint16_temp.empty())
                  {
                     Data_uint16_temp.pop_back();
                  }
                cout << "iteration: " << j << endl;
            }
            outputFile.close();
        }
        else
        {
            std::cout << "File name: " << inputFileName << "is not supported, add int16 or uint16 to the name to indicate fill type"  << '\n';
            outputFile.close();
        }
    }
    /* printing a list of the compression rates for data analysis */
    cout << "List of CR rates:" << endl;
    for(int i = 2 ; i < totalFiles ; ++i)
    {

        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        cout << CRforPrinting.at(i-2) << endl;
    }
    return 0;
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

/* this function should take a 265 x dataColumns long vector and compress it */
std::vector<uint8_t> FELACS (std::vector<uint16_t> Data, int blocksize, int N, int dataColumns)
{
    int totalBlocks = int (floor( double (Data.size()) / blocksize));
    std::vector<uint16_t> block(blocksize);
    std::vector<int32_t> diffBlock(blocksize-1);
    std::vector<uint32_t> diffBlockConv(blocksize-1);
    std::vector<uint8_t> compressed;
    /* Debugging purposes only */
    ostringstream os_filePath;
    string outputPath = "";
    os_filePath << "C:/Users/basti/OneDrive/THESIS/2_Compression_Algorithms/C++/Matlab_Simulations/Distribution_of_Si.txt";
    outputPath =  os_filePath.str();
    std::ofstream outputFile(outputPath, std::ios::out | std::ofstream::binary | std::ios_base::app);
    std::vector<int> si_Occuring(N); // for testing purposes!!
    uint16_t firstSample;
    int32_t omega;
    uint16_t k;
    uint16_t k_optimal;
    uint64_t D;
    uint8_t temp;
    uint64_t filledBits;
    int si;
    uint16_t ai;
    uint8_t lengthai;
    int extraBits = 0;
    // loop through all dataColumns
    for(int i = 0 ; i < dataColumns ; ++i)
    {
        // split data in blocks of size 264 samples (e.g. 528 bytes)
        for(int j = 0 ; j < blocksize ; ++j)
        {
            block.at(j) = Data.at(j + i*dataColumns);
        }
        // initialize first sample
        firstSample = block.at(0);
        // calculate differentiated values
        for(int j = 0 ; j < blocksize - 1 ; ++j)
        {
            diffBlock.at(j) = int32_t(int32_t(block.at(j + 1)) - int32_t(block.at(j)));
        }


        // use mapping function to only have positive values
        for(uint64_t j = 0; j < diffBlock.size() ; ++j)
        {
            omega = int32_t(std::min(int32_t(block.at(j)), int32_t(pow(2,N) - 1 - block.at(j))));
//             if(j + i*blocksize > 28300 && j + i*blocksize < 28500)
//                cout << (j + i*blocksize) << " |      omega: " << omega << endl;

            if(0 <= diffBlock.at(j) && diffBlock.at(j) <= omega)
            {
                diffBlockConv.at(j) = 2*diffBlock.at(j);
            }
            else if(-omega <= diffBlock.at(j) && diffBlock.at(j) < 0)
            {
                diffBlockConv.at(j) = 2*abs(diffBlock.at(j)) - 1;
            }
            else
            {
                diffBlockConv.at(j) = omega + uint32_t(abs(diffBlock.at(j)));
            }

        }
        // calculate optimum value of k using the value D
        D = 0;
        for(uint64_t j = 0 ; j < diffBlockConv.size() ; ++j)
        {
            D += diffBlockConv.at(j);
        }
        k = 0;
        if(D < 2*diffBlockConv.size())
            k_optimal = k;
        else
        {
            for(k = 1 ; k < 7 ; ++k)
            {
                if(pow(2,k) * diffBlockConv.size() < D && D <= pow(2,k+1)*diffBlockConv.size())
                {
                    k_optimal = k;
                }
            }
            if(128 * diffBlockConv.size() < D)
            {
                k_optimal = 7;
            }
        }
        // create block packet [firstample, k-value, diffBlock]
        compressed.push_back(firstSample);
        compressed.push_back(firstSample >> 8);
        // save k_optimal with 3 bits
        filledBits = 0;
        temp = k_optimal;
        filledBits += 3;
        // save rest of the data points in structure << si, ai >>
        for(uint64_t j = 0 ; j < diffBlockConv.size() ; ++j)
        {
            // log(0) will give an error, thus diffBlockConv should be > 0.
            // next calculate the value of si and store these in compressed
            if(diffBlockConv.at(j) > 0)
            {
                si = floor(log2(diffBlockConv.at(j))) - k_optimal + 2;
                if(si < 1)
                {
                    si = 1;
                }
                for(int k = 0; k < si ; k++)
                {
                    // if si = 3 insert '001', si = 1 insert '1', only '1' has to be inserted
                    // '0' are already intialized
                    if(k == (si - 1))
                    {
                        temp += (1 << (filledBits % 8));
                    }
                    filledBits += 1;
                    // if filledBits is an equal of 8, it means temp is full
                    if(filledBits % 8 == 0)
                    {
                        compressed.push_back(temp);
                        temp = 0;
                    }
                }
            }
            else
            {
                si = 1;
                // si = 1 insert '1'
                temp += (1 << (filledBits % 8));
                filledBits += 1;
                // if filledBits is an equal of 8, it means temp is full
                if(filledBits % 8 == 0)
                {
                    compressed.push_back(temp);
                    temp = 0;
                }
            }
            si_Occuring.at(si) += 1; // debugging purposes!!
            if(si == 1)
            {
                ai = diffBlockConv.at(j);
            }
            else
            {
                ai = diffBlockConv.at(j) - pow(2,k_optimal + si - 2);
            }
            // write ai to compressed
            if(si >= 2 )
            {
                lengthai = k_optimal+si-2;
            }
            else
            {
                lengthai = k_optimal;
            }
            for(uint16_t k = 0 ; k < lengthai ; k++ )
            {
                // check if k's bit of ai is 0 or 1
                if((ai & (1 << k)) != 0)
                {
                    temp += (1 << (filledBits % 8));
                }
                filledBits += 1;
                // if filledBits is an equal of 8, it means temp is full
                if(filledBits % 8 == 0)
                {
                    compressed.push_back(temp);
                    temp = 0;
                }
            }
        }
        // push last value back and set temp to 0;

        if((filledBits % 8) != 0)
        {
            extraBits += 8 - (filledBits % 8);
            compressed.push_back(temp);
        }
        if((filledBits % 8) != 0)
        {
//            cout << ((filledBits + 8 - (filledBits % 8))/ 8) << endl;
        }
        else
//            cout << (filledBits / 8) << endl;
        temp = 0;
    }

    for(int k = 0 ; k < si_Occuring.size(); ++k)
    {
        outputFile.write(reinterpret_cast<const char *>(&si_Occuring[k]), sizeof(si_Occuring[k]));
        //cout << k << " | " << si_Occuring.at(k) << endl;
        cout << si_Occuring.at(k) << endl;
    }
    outputFile.close();
    std::cerr << "Out of Felacs" << endl;
    cout << "extra-bits " << extraBits << endl;
    return compressed;
    // create package
}

std::vector<uint16_t> temperatureConversion(std::vector<double> Temp_F)
{
    // constant vectors
    std::vector<uint16_t> Temp_Int16(Temp_F.size());
    double d1 = -40;
    double d2 = 0.01;

    for(uint32_t i = 0; i < Temp_F.size(); ++i)
    {
        Temp_Int16.at(i) = uint16_t (round((Temp_F.at(i) - d1)/d2));
    }
    return Temp_Int16;
}

std::vector<uint64_t> pdf(std::vector<uint16_t> Data, int byteRange)
{
    std::vector<uint64_t> probabilityfunction(pow(2,byteRange), 0);
    // increase elements, which are found
    for(uint64_t i = 0; i < Data.size() ; ++i)
    {
        probabilityfunction.at(Data.at(i)) += 1;
    }
    return probabilityfunction;
}

double entropy(std::vector<uint64_t> Data)
{
    double p = 0;
    double entr = 0;
    double sumData = 0;
    // convert frequency to probability range [0, 1)
    for(uint64_t i = 0; i < Data.size(); ++i)
    {
        sumData += Data[i];
    }
    // calculate entropy
    for(uint64_t i = 0; i < Data.size();  ++i)
    {
        p = double(Data[i])/sumData;
        if(p > 0)
        {
            entr -= p*log2(p);
        }
    }
    return entr;
}
