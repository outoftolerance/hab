#include "DataLog.h"

/*------------------------------Constructor Methods------------------------------*/
DataLog::DataLog(int chip_select) :
  chip_select_(chip_select)
{

}

/*------------------------------Public Methods------------------------------*/
bool DataLog::init(const String& filename, const String& header)
{
	filename.toCharArray(filename_, MAX_FILENAME_LENGTH);
	header.toCharArray(header_, MAX_HEADER_LENGTH);

	if (!SD.begin(chip_select_)) {
    	return false;
  	}

  	log_file_ = SD.open(filename_, FILE_WRITE);

  	if(!log_file_)
  	{
  		return false;
  	}

  	log_file_.println(header_);

  	log_file_.close();

	return true;
}

bool DataLog::entry(const float data[], int size, bool newline)
{
	int i;
	String data_string;

	for(i = 0; i < size; i++)
	{
		data_string += String(data[i]);

		if(i == size - 1 && newline == false)
		{
			data_string += ",";
		}
		else if(i < size - 1)
		{
			data_string += ",";
		}
	}

	log_file_ = SD.open(filename_, FILE_WRITE);

	if(newline)
	{
		log_file_.println(data_string);
	}
	else
	{
		log_file_.print(data_string);
	}

	log_file_.close();
}

bool DataLog::entry(const int data[], int size, bool newline)
{
	int i;
	String data_string;

	for(i = 0; i < size; i++)
	{
		data_string += String(data[i]);

		if(i == size - 1 && newline == false)
		{
			data_string += ",";
		}
		else if(i < size - 1)
		{
			data_string += ",";
		}
	}

	log_file_ = SD.open(filename_, FILE_WRITE);

	if(newline)
	{
		log_file_.println(data_string);
	}
	else
	{
		log_file_.print(data_string);
	}

	log_file_.close();
}