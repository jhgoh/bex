#ifndef ConfigReader_H
#define ConfigReader_H

#include <boost/format.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <map>

class ConfigReader
{
public:
  ConfigReader(const char* fileName, int argc, char* argv[]);

  void processInputCommand(const std::string line);

  void print(std::ostream& out = std::cout) const;
  bool hasOption(const std::string name) const;
  // Case insensitive map, from http://stackoverflow.com/questions/1801892/making-mapfind-operation-case-insensitive
  struct ci_less : std::binary_function<std::string, std::string, bool>
  {
    // case-independent (ci) compare_less binary function
    struct nocase_compare : public std::binary_function<unsigned char,unsigned char,bool>
    {
      bool operator() (const unsigned char& c1, const unsigned char& c2) const
      {
          return tolower(c1) < tolower(c2);
      }
    };
    bool operator() (const std::string & s1, const std::string & s2) const
    {
      return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), nocase_compare());  // comparison
    }
  };

  typedef std::map<std::string, std::string, ci_less> DataMap;
  typedef std::map<std::string, int, ci_less> MenuType;

  // Default getter
  template<typename T>
  T get(const std::string name) const
  {
    if ( !hasOption(name) )
    {
      throw std::out_of_range("Cannot find config name " + name);
    }

    T value;
    std::stringstream ss(data_.at(name));
    ss >> value;

    return value;
  }

  // Algernative getter, raise exception if value is not in range
  template<typename T>
  T get(const std::string name, const T minValue, const T maxValue) const
  {
    const T value = get<T>(name);
    if ( value < minValue ) throw std::underflow_error((boost::format("%1% must not less than %3% (was %2%)") % name % value % minValue).str());
    if ( value > maxValue ) throw std::overflow_error((boost::format("%1% must not exceed %3% (was %2%)") % name % value % maxValue).str());
    return value;
  }

  // Alternative getter, choose from menu items
  int get(const std::string name, const MenuType& itemMap) const;

  // Generic setter with value type
  template<typename T>
  void set(const std::string name, const T& value)
  {
    data_[name] = (boost::format("%1%") % value).str();
  }

private:
  DataMap data_;
};

template<> std::vector<double> ConfigReader::get(const std::string name) const;

#endif

