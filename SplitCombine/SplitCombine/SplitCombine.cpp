#include "cxxopts.hpp"
#include <iostream>
#include <fstream>

using namespace  std;

int Split(const string& fileName, long long block_size)
{
	int blockNo = 0;
	ifstream infile(fileName, ios::binary|ios::ate);
	if (!infile.is_open())
	{
		cerr << "File not found" << endl;
		return EXIT_FAILURE;
	}
	long long remain = infile.tellg();
	const long long size = remain;
	infile.seekg(0, ios::beg);
	while (!infile.eof())
	{
		vector<char> result(block_size);
		infile.read(&result[0], block_size);
		if (!infile)
		{
			result.resize(infile.gcount());
		}
		ostringstream os;
		os << fileName << '.' << ++blockNo;
		ofstream outfile(os.str(), ios::binary);
		outfile.write(&result[0], result.size());
		remain -= result.size();
		cout << "Done " << os.str() << " (" << 100*remain/size << "% remaining)" << endl;
	}
	return 0;
}

int Combine(const string& fileName)
{
	int blockNo = 0;
	ofstream outfile(fileName, ios::binary);
	while (true)
	{
		ostringstream os;
		os << fileName << '.' << ++blockNo;
		ifstream infile(os.str(), ios::binary | ios::ate);
		cout << os.str() << " ";
		if (!infile.is_open())
		{
			cout << "Not found" << endl;
			return 0;
		}
		long long size = infile.tellg();
		vector<char> result(size);
		infile.seekg(0, ios::beg);
		infile.read(&result[0], size);
		outfile.write(&result[0], size);
		cout << "Done" << endl;
	}
	
}

int main(int argc, char *argv[])
{
	try {
		cxxopts::Options options(argv[0], "Split/Combine app");

		options.add_options("")
			("s,split", "split mode", cxxopts::value<bool>())
			("c,combine", "combine mode", cxxopts::value<bool>())
			("h,help", "help", cxxopts::value<bool>())
			("file", "file", cxxopts::value<string>())
			("b,block", "block size", cxxopts::value<string>()->default_value("10M"));
		options.parse_positional({ "split","combine","help" });
		auto result = options.parse(argc, argv);
		if (result.count("help"))
		{
			cout << options.help({ "" }) << endl;
			return 0;
		}
		if (result.count("split") + result.count("combine") == 0)
		{
			cerr << "Mode are not specified" << endl;
			return EXIT_FAILURE;
		}
		if (result.count("split") + result.count("combine") > 1)
		{
			cerr << "More than one mode are specified" << endl;
			return EXIT_FAILURE;
		}
		if (!result.count("file"))
		{
			cerr << "File are not specified" << endl;
			return EXIT_FAILURE;
		}
		string file = result["file"].as<string>();

		if (result.count("split"))
		{
			string block = result["block"].as<string>();
			istringstream ss(block);
			long long blockSize;
			ss >> blockSize;
			if (!ss.eof())
			{
				char suffix;
				ss >> suffix;
				switch (toupper(suffix))
				{
				case 'K':
					blockSize *= 1024;
					break;
				case 'M':
					blockSize *= 1024 * 1024;
					break;
				case 'G':
					blockSize *= 1024 * 1024 * 1024;
					break;
				default:
					cerr << "Bad block size" << endl;
					return EXIT_FAILURE;
				}
			}
			return Split(file, blockSize);
		}
		return Combine(file);
	}
	catch (const cxxopts::OptionException& e)
	{
		cout << "error parsing options: " << e.what() << endl;
		exit(1);
	}
	return 0;
}

