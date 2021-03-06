#include "cxxopts.hpp"
#include <iostream>
#include <fstream>


using namespace  std;

const size_t block_size = 64*1024;

int Split(const string& fileName, long long part_size, size_t parts)
{
	int blockNo = 0;
	ifstream infile(fileName, ios::binary | ios::ate);
	if (!infile.is_open())
	{
		cerr << "File not found" << endl;
		return EXIT_FAILURE;
	}
	long long remain = infile.tellg();
	const long long size = remain;
	infile.seekg(0, ios::beg);
	if (part_size == -1)
	{
		part_size = size / parts + 1;
	}
	vector<char> result(block_size);

	while (!infile.eof())
	{
		ostringstream os;
		os << fileName << '.' << ++blockNo;
		ofstream outfile(os.str(), ios::binary);
		long long processed = 0;
		while (!infile.eof() && processed < part_size)
		{
			const size_t rd_size = min(block_size, static_cast<size_t>(part_size - processed));
			infile.read(&result[0], rd_size);
			const size_t bl_processed = infile ? rd_size : infile.gcount();
			outfile.write(&result[0], bl_processed);
			processed += bl_processed;
		}
		remain -=  part_size;
		cout << "Done " << os.str() << " (" << 100 * remain / size << "% remaining)   " << "\r";
	}
	cout << endl;
	return 0;
}

int Combine(const string& fileName)
{
	int blockNo = 0;
	ofstream outfile(fileName, ios::binary);
	vector<char> result(block_size);
	while (true)
	{
		ostringstream os;
		os << fileName << '.' << ++blockNo;
		ifstream infile(os.str(), ios::binary);
		cout << os.str() << " ";
		if (!infile.is_open())
		{
			cout << "Not found. Process complete" << endl;
			return 0;
		}
		while (!infile.eof())
		{
			infile.read(&result[0], block_size);
			const size_t readed = infile ? block_size : infile.gcount();
			outfile.write(&result[0], readed);
		}
		cout << "Done  \r";
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
			("b,block", "block size", cxxopts::value<string>()->default_value("10M"))
		    ("p,parts", "auto calc size to do n parts", cxxopts::value<size_t>());
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
		const string file = result["file"].as<string>();

		if (result.count("split"))
		{
			long long blockSize;
			size_t parts = 0;
			if (result.count("parts"))
			{
				blockSize = -1;
				parts = result["parts"].as<size_t>();
			}
			else
			{
				const string block = result["block"].as<string>();
				istringstream ss(block);
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
			}
			return Split(file, blockSize, parts);
		}
		return Combine(file);
	}
	catch (const cxxopts::OptionException& e)
	{
		cout << "error parsing options: " << e.what() << endl;
		exit(1);
	}
}

