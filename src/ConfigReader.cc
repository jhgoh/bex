#include "include/ConfigReader.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>

using namespace std;
using namespace boost::algorithm;

ConfigReader::ConfigReader(const char* fileName, int argc, char* argv[])
{
  ifstream fin(fileName);
  if ( fin )
  {
    string line;
    while ( getline(fin, line) )
    {
      const size_t commentPos = line.find('#');
      if ( commentPos != string::npos) line.erase(commentPos);
      trim(line);
      if ( line.empty() or line[0] == '#' ) continue;
      processInputCommand(line);
    }
  }
  else
  {
    throw runtime_error(string("Cannot open file") + fileName);
  }
  for ( int i=0; i<argc; ++i )
  {
    processInputCommand(argv[i]);
  }
}

void ConfigReader::processInputCommand(const string line)
{
  const size_t assignPos = line.find('=');
  if ( assignPos == string::npos )
  {
    cerr << "!!ConfigReader: invalid input command \"" << line << "\". we skip this input\n";
    return;
  }

  string name = line.substr(0, assignPos);
  string value = line.substr(assignPos+1);
  trim(name);
  trim(value);

  data_[name] = value;
}

bool ConfigReader::hasOption(const std::string name) const
{
  return data_.find(name) != data_.end();
}

void ConfigReader::print() const
{
  // Print out configurations
  for ( int i=0, n=42; i<n; ++i ) cout << "#";
  cout << "\n## ConfigReader:Printing out parameters ##\n";
  for ( std::map<string, string>::const_iterator key = data_.begin();
        key != data_.end(); ++key )
  {
    cout << boost::format("## %1% %|20t| = %2% %|40t|##\n") % key->first % key->second;
  }
  for ( int i=0, n=42; i<n; ++i ) cout << "#";
  cout << endl;
}

template<>
std::vector<double> ConfigReader::get(const std::string name) const
{
  if ( !hasOption(name) )
  {
    throw std::out_of_range("Cannot find config name " + name);
  }
  std::string valueStr = data_.at(name);
  std::replace(valueStr.begin(), valueStr.end(), ',', ' ');

  std::vector<double> l;
  std::stringstream ss(valueStr);
  double val;
  while ( ss>>val ) l.push_back(val);

  return l;
}

int ConfigReader::get(const std::string name, const ConfigReader::MenuType& itemMap) const
{
  const std::string value = get<std::string>(name);
  int index;
  try
  {
    index = itemMap.at(value);
  }
  catch ( exception& e )
  {
    cerr << "!!! Cannot find menu item for " << name << endl;
    cerr << "!!! Choose among ";
    for ( ConfigReader::MenuType::const_iterator item = itemMap.begin();
          item != itemMap.end(); ++item )
    {
      cerr << item->first << ":" << item->second << " ";
    }
    cerr << endl;
    throw e;
  }
  return index;
}

