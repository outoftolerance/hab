#include "DataLog.h"

/*------------------------------Constructor Methods------------------------------*/
DataLog::DataLog(const char filename[], const char header[], int chip_select) :
  filename_(filename),
  header_(header),
  chip_select_(chip_select)
{

}

/*------------------------------Public Methods------------------------------*/
bool DataLog::init()
{
	if (!SD.begin(chip_select_)) {
    	return false;
  	}

  	log_file_ = SD.open(filename_, FILE_WRITE);

  	if(!log_file_)
  	{
  		return false;
  	}

  	log_file.println(header_);

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

	if(newline)
	{
		log_file_.println(data_string);
	}
	else
	{
		log_file_.print(data_string);
	}
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

	if(newline)
	{
		log_file_.println(data_string);
	}
	else
	{
		log_file_.print(data_string);
	}
}