clc;
clear all;

filename = 'C:\Users\basti\OneDrive\THESIS\2_Compression_Algorithms\Algorithms\C++\CodeBlocks\FELACS\Temp_LU_84_PDF_Data_Raw.txt';

fileID = fopen(filename, 'r', 'ieee-le');
if fileID == -1, error('Cannot open file: %s', filename); end
format = 'uint64';
DataRead = fread(fileID, format);
fclose(fileID);



plot(DataRead);