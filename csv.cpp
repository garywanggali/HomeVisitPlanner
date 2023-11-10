# include "csv.h"
#include "iostream"

using namespace std;

int main(){
  io::CSVReader<2> in("add.csv");
  in.read_header(io::ignore_extra_column, "no", "address");
  int no; 
  string address; 
  while(in.read_row(no, address)){
    // do stuff with the data
    cout <<no<<","<< address << endl;
  }
}