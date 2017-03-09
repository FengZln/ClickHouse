#include <string>

#include <iostream>
#include <iomanip>

#include <DB/Core/Types.h>
#include <DB/Common/Stopwatch.h>
#include <DB/IO/WriteBufferFromFile.h>
#include <DB/IO/ReadBufferFromFile.h>
#include <DB/IO/ZlibDeflatingWriteBuffer.h>
#include <DB/IO/ZlibInflatingReadBuffer.h>
#include <DB/IO/WriteHelpers.h>
#include <DB/IO/ReadHelpers.h>


int main(int argc, char ** argv)
{
	try
	{
		std::cout << std::fixed << std::setprecision(2);

		size_t n = 100000000;
		Stopwatch stopwatch;

		{
			DB::WriteBufferFromFile buf("test_zlib_buffers.gz", DBMS_DEFAULT_BUFFER_SIZE, O_WRONLY | O_CREAT | O_TRUNC);
			DB::ZlibDeflatingWriteBuffer deflating_buf(buf, DB::ZlibCompressionMethod::Gzip, /* compression_level = */ 3);

			stopwatch.restart();
			for (size_t i = 0; i < n; ++i)
			{
				DB::writeIntText(i, deflating_buf);
				DB::writeChar('\t', deflating_buf);
			}
			deflating_buf.finish();

			stopwatch.stop();
			std::cout << "Writing done. Elapsed: " << stopwatch.elapsedSeconds() << " s."
				<< ", " << (deflating_buf.count() / stopwatch.elapsedSeconds() / 1000000) << " MB/s"
				<< std::endl;
		}

		{
			DB::ReadBufferFromFile buf("test_zlib_buffers.gz");
			DB::ZlibInflatingReadBuffer inflating_buf(buf, DB::ZlibCompressionMethod::Gzip);

			stopwatch.restart();
			for (size_t i = 0; i < n; ++i)
			{
				size_t x;
				DB::readIntText(x, inflating_buf);
				inflating_buf.ignore();

				if (x != i)
					throw DB::Exception("Failed!, read: " + std::to_string(x) + ", expected: " + std::to_string(i));
			}
			stopwatch.stop();
			std::cout << "Reading done. Elapsed: " << stopwatch.elapsedSeconds() << " s."
				<< ", " << (inflating_buf.count() / stopwatch.elapsedSeconds() / 1000000) << " MB/s"
				<< std::endl;
		}
	}
	catch (const DB::Exception & e)
	{
		std::cerr << e.what() << ", " << e.displayText() << std::endl;
		return 1;
	}

	return 0;
}
