#include "FileProcessor.h"

FileProcessor::FileProcessor(std::vector<std::wstring> & p) : paths(p)
{
	logfile = extractPath(p[0]) + L"\\log.txt";
}

void FileProcessor::log()
{
	sort(paths.begin(), paths.end());
	checksums.resize(paths.size());

	initialWrite(getMaxWidth() + additionalSpace);

	unsigned int currentAmountOfCores = std::thread::hardware_concurrency();
	ThreadPool threadPool(minAmountOfThreads > currentAmountOfCores ? 
		minAmountOfThreads : currentAmountOfCores);

	threadPool.enqueue( [=]() { threadIO(); } );
	for (size_t i = 0; i < paths.size(); i++)
	{
		checksums[i] = threadPool.enqueue( [=] () { return threadSum(paths[i]); } );
	}
	
}

void FileProcessor::threadIO()
{
	std::vector<bool> doneTasks(paths.size());
	std::wfstream stream;
	stream.open(logfile, std::ios::in|std::ios::out);
	int counter = 0;
	int linesToSkip = 3;

	while (counter != checksums.size())
	{
		for (size_t i = 0; i < checksums.size(); i++)
		{
			if (doneTasks[i] || !checksums[i]._Is_ready())
			{
				continue;
			}

			int checksum = checksums[i].get();
			unsigned currentLine = 0;

			stream.seekg(0, std::ios_base::beg);

			while (currentLine < i + linesToSkip)
			{
				stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				++currentLine;
			}

			std::streamoff position = stream.tellg();
			stream.seekp(position);
			stream << std::hex << checksum;

			doneTasks[i] = true;
			counter++;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(defaultSleepTime));
	}
	stream.close();
}

int FileProcessor::threadSum(const std::wstring path)
{
	boost::crc_32_type result;
	std::ifstream ifs(path, std::ios_base::binary);

	if (ifs)
	{
		do
		{
			char buffer[buffer_size];
			ifs.read(buffer, buffer_size);
			result.process_bytes(buffer, ifs.gcount());
		} while (ifs);
	}

	return result.checksum();
}

void FileProcessor::initialWrite(unsigned int nameWidth)
{
	std::wofstream out;
	out.imbue(std::locale(std::locale::empty(), 
		new (std::nothrow) std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
	out.open(logfile);
	
	if (out.is_open())
	{
		out << std::right << std::setfill(L'-') << std::setw(nameWidth + 45) << "+" << std::endl;
		out << std::setfill(L' ') << std::left << std::setw(10) << "Checksum"
			<< std::setw(nameWidth) << "Name"
			<< std::setw(20) << "Size(Mbytes)"
			<< std::setw(15) << "Creation Date" << std::endl;
		out << std::right << std::setfill(L'-') << std::setw(nameWidth + 45) << "+" << std::endl;
		out << std::setfill(L' ');

		for (size_t i = 0; i < paths.size(); i++)
		{
			out << std::left << std::wstring(checksumWidth, ' ') << std::setw(nameWidth) << extractName(paths[i])
				<< std::setw(20) << std::fixed << std::setprecision(4) 
				<< WinInfoGetter::fileSize(paths[i])
				<< WinInfoGetter::getCreationDate(paths[i]) << std::endl;
		}
		out << std::right << std::setfill(L'-') << std::setw(nameWidth + 45) << "+" << std::endl;
		out.close();
	}
}

unsigned int FileProcessor::getMaxWidth()
{
	std::vector<std::wstring>::iterator longest = max_element(paths.begin(),
		paths.end(), compareLength);
	return extractName(*longest).size();
}

bool compareLength(const std::wstring & lhs, const std::wstring & rhs) 
{
	return lhs.size() < rhs.size();
}

std::wstring extractName(const std::wstring & path)
{
	return std::wstring(find_if(path.rbegin(), path.rend(),
		[](char c) { return c == '/' || c == '\\'; }).base(),
		path.end());
}

std::wstring extractPath(const std::wstring & path)
{
	return std::wstring(path.begin(), find_if(path.rbegin(), path.rend(),
		[](char c) { return c == '/' || c == '\\'; }).base());
}