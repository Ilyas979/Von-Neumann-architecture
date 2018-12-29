#include <map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <fstream>
#include <vector>
#include <locale>
#include "code.h"
using namespace std;
static void delete_spaces(string & s) {
	if (s[s.size()-1] == '\r') s.erase(s.size() - 1, s.size());
	while (s[0] == ' ') s.erase(0, 1);
	string::size_type pos = s.find('%');
	if (pos != string::npos) s.erase(pos, string::npos);
}

static string process(string & s)
{
	delete_spaces(s);
	//output is the same as s-tring but for sure without CR (\r)
    string::size_type pos = s.find(' ');
    string s1;
    if (pos != string::npos) {
      s1 = s.substr(0, pos);
      s = s.substr(pos + 1, s.size() );
      return s1;
    }
    else {
        return s;
    }
}
static unsigned int type_fill(string & s)
{
	unsigned int k;
    string::size_type pos = s.find(' ');
    string s1;
    if (pos != string::npos) {
      s1 = s.substr(0, pos);
      s = s.substr(pos + 1, s.length() );
      if (pos == 1) k = s1[0] - '0';
      if (pos == 2) k = 10*(s1[0] - '0') + s1[1] - '0';
      return k;
    }
    else {
		s = "";
        return 71;
    }
}
static unsigned int int_to_uint(int a) {
	const unsigned int C19 = 524288;
	unsigned int b = 0;
    if (a < 0) {
        b = (unsigned int)(~(a - 1)) + C19 - 1;
    }
    else  b = (unsigned int)a;
	return b;
}
static int uint_to_int(unsigned int a) {
	const unsigned int C19 = 524288;
	int b;
	if (a >= C19) {
		b = (~((int)(a - (C19-1)))) + 1;
	}
	else b = (int)a;
	return b;
}
static int uint_to_int_C15(unsigned int a) {
	const unsigned int C15 = 1<<15;
	int b;
	if (a >= C15) {
		b = (~((int)(a - (C15-1)))) + 1;
	}
	else b = (int)a;
	return b;
}
static unsigned int com (unsigned int mem) {
	return (mem>>24);
}
static unsigned int reg_1 (unsigned int mem) {
//	if ( vec_type[com(mem)] == J ) cout << "error: no register in type J\n";
/*	else*/ return ((mem<<8)>>28);
}
static unsigned int reg_2 (unsigned int mem) {
//	if ( vec_type[com(mem)] != RR ) cout << "error: no register in types J, RM, RI\n";
/*	else*/ return ((mem<<12)>>28);
}
static unsigned int Imm (unsigned int mem) {
  return mem&0xFFFFF;
}
static unsigned int Imm_RR (unsigned int mem) {
	return mem&0xFFFF;
}

class CPU {
  private:
	static const unsigned int NumberOfOperations = 52;
	static const unsigned int ComandsSize = 72;
	static const unsigned int RegistersSize = 17;
  static const unsigned int C20 = 1048576;
  static const unsigned int C15 = 32768;
  static const unsigned int C19 = 524288;
  unsigned int mem[C20];
  int reg1[RegistersSize];
	vector <string> r;
	string comands[ComandsSize];
  map <string, int> labels;
  map <string, int> labels1;
  map <unsigned int, string> l;
	map <unsigned int, string> var_s;
	vector <string> var;
  map <string, unsigned int> codes;
  map <string, int> reg;
  vector <unsigned int> vec_type;
  unsigned int n; //number of lines in assembler code without labels
  unsigned int DataSize;
  unsigned int ConstSize;
  static const unsigned int StartStackPointer;
  unsigned int ExecPointer;
  fstream F;
  static const unsigned int CodePointer = 512;
  union doubit {
		long long a;
		unsigned long long u;
		double d;
	};
	union intuint {
		unsigned long long u;
		long long s;
	};
	intuint res;
  doubit doub;
  doubit doub2;
	enum d {
	   RR = 1, RM = 2, RI = 3, J = 4
	};

	int correct_address(string str);

  public:
  CPU();
  string FileName;
  string ExecFileName;
  string AssemblyFileName;
  void ReadLabels();
	void ReadComands();
	void MakeFbin();
  void MakeExecFile();
  void RunExecFile();
  void Disassembler();
	void Execution();
	void FillOfMemFromExe(ifstream &Fin);
};
const unsigned int CPU::StartStackPointer = C20 - 1;

CPU::CPU()
{
	struct sinit_cmd {
	   string opname;
	   unsigned opcode;
	   d type;
	};
	typedef struct sinit_cmd init_cmd;
	init_cmd com[NumberOfOperations] = {
	   {"halt", HALT, RI},
	   {"syscall", SYSCALL, RI},
		 {"add", ADD, RR},
		 {"addi", ADDI, RI},
		 {"sub", SUB, RR},
		 {"subi", SUBI, RI},
		 {"mul", MUL, RR},
		 {"muli", MULI, RI},
		 {"div", DIV, RR},
		 {"divi", DIVI, RI},
		 {"lc", LC, RI},
		 {"shl", SHL, RR},
		 {"shli", SHLI, RI},
		 {"shr", SHR, RR},
		 {"shri", SHRI, RI},
		 {"and", AND, RR},
		 {"andi", ANDI, RI},
		 {"or", OR, RR},
		 {"ori",ORI, RI},
		 {"xor", XOR, RR},
		 {"xori", XORI, RI},
		 {"not", NOT, RI},
		 {"mov", MOV, RR},
		 {"addd", ADDD, RR},
		 {"subd", SUBD, RR},
		 {"muld", MULD, RR},
		 {"divd", DIVD, RR},
		 {"itod", ITOD, RR},
		 {"dtoi", DTOI, RR},
		 {"push", PUSH, RI},
		 {"pop", POP, RI},
		 {"call", CALL, RR},
		 {"calli", CALLI, J},
		 {"ret", RET, RI},
		 {"cmp", CMP, RR},
		 {"cmpi", CMPI, RI},
		 {"cmpd", CMPD, RR},
		 {"jmp", JMP, J},
		 {"jne", JNE, J},
		 {"jeq", JEQ, J},
		 {"jle", JLE, J},
		 {"jl", JL, J},
		 {"jge", JGE, J},
		 {"jg", JG, J},
		 {"load", LOAD, RM},
		 {"store", STORE, RM},
		 {"load2", LOAD2, RM},
		 {"store2", STORE2, RM},
		 {"loadr", LOADR, RR},
		 {"storer", STORER, RR},
		 {"loadr2", LOADR2, RM},
		 {"storer2", STORER2, RR}
	};
	vec_type.resize(ComandsSize);
	for (unsigned int i = 0; i < NumberOfOperations; i++) {
	  comands[com[i].opcode] = com[i].opname;
		codes.insert(pair<string, unsigned int>(com[i].opname, com[i].opcode));
		vec_type[com[i].opcode] = com[i].type;
	}
	r.resize(RegistersSize - 1);
  for (int i = 0; i < RegistersSize - 1; i++) { //fill vector of register names
		r[i] = "r";
		if (i >= 10) r[i] += '1';
		r[i] += '0' + i%10;
  }
  reg ["r0"] = 0;
  reg ["r1"] = 1;
  reg ["r2"] = 2;
  reg ["r3"] = 3;
  reg ["r4"] = 4;
  reg ["r5"] = 5;
  reg ["r6"] = 6;
  reg ["r7"] = 7;
  reg ["r8"] = 8;
  reg ["r9"] = 9;
  reg ["r10"] = 10;
  reg ["r11"] = 11;
  reg ["r12"] = 12;
  reg ["r13"] = 13; //frame call pointer
  reg ["r14"] = 14; //stack pointer
  reg ["r15"] = 15; //program counter
  reg ["flags"] = 16; //cmp result
  for (unsigned int i = 0; i < C20; i++) {
	   mem[i] = 0;
  }
  for (unsigned int i = 0; i < RegistersSize; i++) {
	   reg1[i] = 0;
  }
  reg1[14] = C20 - 1;
}
int CPU::correct_address(string str) {// Distinguishes wheather the adress is a number or label
	unsigned int i = 0;
	while (i < str.length()) {
		if (!(0 <= str[i]-'0' && str[i]-'0' <= 9)) {
			return 0;
		}
		i++;
	}
	//if (stoul(str, nullptr, 0) > DataSize + n) return 0; SELF-MODIFIABLE CODE - should not set boundaries for adress
	return 1;
}
void CPU::ReadLabels() {
	  DataSize = 0;
	  string str, strorig;
	  string word;
	  size_t found;
	  unsigned int i = 0;
	  unsigned int curline = 1;
	  n = 0;
	  getline(F, str);
		delete_spaces(str);
	  strorig = str;
	  while (!F.eof()) {
		  found = strorig.find_first_of(":");
		  if (found != std::string::npos) {		//Case when there is a label or variable in line
			  if (found == strorig.size() - 1) {
					labels1[strorig] = i;
			    str.erase(str.end()-1, str.end());
			    if (str == "main") ExecPointer = i;
			    labels[str] = i;
			  }
			  else {
			    word = strorig.substr(strorig.find(" ") + 1);
			    if(word == "word") {
					labels1[strorig.substr(0, found + 1)] = i+1;
					labels[strorig.substr(0, found)] = i+1;
					DataSize++;
					i++;
				}
			  }
		  }
		  else if (strorig == "end main\r" || strorig == "end main") { //Case when there is "end main" in line
			  labels["end main"] = i;
				break;																										//Do not read after "end main"
		  }
		  else if (codes.find(process(str)) != codes.end()) { 	//Case when there is a command name in line
		    i++;
		    n++;
		  }
		  else {
				string str = to_string(curline);
				str = "in line = " + str + " there is smth wrong";
				char* c = new char[str.length()+1];
				strcpy(c, str.c_str());
				throw c;
			}
		  curline++;
		  getline(F, str);
		  delete_spaces(str);
		  strorig = str;
	  }
	  F.clear();
	  F.seekg(0, ios::beg);

//  cout << "main - > " << labels["main"] << endl;
//cout << "skip0 -> " << labels1["skip0:"] << endl;
//  cout << "sqr -> " << labels["sqr"] << endl;
//  cout << "fact -> " << labels["fact"] << endl;
//  cout << "x -> " << labels["x"] << endl;
//  cout << "y -> " << labels["y"] << endl;
//  cout << "z -> " << labels["z"] << endl;
//  cout << "end main -> " << labels["end main"] << endl;
}
void CPU::ReadComands() {
	string substr1, substr2, substr3, substr4, str;
	unsigned int i = 0;
	unsigned int num = 0;
	int num2 = 0;
	while(!F.eof()) {
		getline(F, str);
		delete_spaces(str); //Here you can also write a condition for "end main"
		if (str.compare(0, 8, "end main") == 0) break;
		substr1 = process(str); //Change process so if argument has a single word return an empty string
		if (labels1.find(substr1) == labels1.end()) {
			num = 0;
	//========================================================================================================================
			if (vec_type[codes[substr1]] == RM) {
				substr2 = process(str);
				substr3 = process(str);
	//	    cout << "substr1 = " << substr1 << " substr2 = " << substr2 << " substr3 = " << substr3  << endl;
	//------------------------------------------------------------------------- Analyze ADDRESS
				if (labels.find(substr3) != labels.end()) num = labels[substr3];
				else if (correct_address(substr3)) {num = stoul(substr3,nullptr, 0);} //for (unsigned int j = 0; j < substr3.length(); ++j){
				else {																																			 //num = num*10 + substr3[j] - '0';}
					string str = to_string(i + 1);
					str = "in line = " + str + " - incorrect address";
					char* c = new char[str.length()+1];
					strcpy(c, str.c_str());
					throw c;
				}
	//------------------------------------------------------------------------- Analyze COMAND
				if(reg.find(substr2) != reg.end()) { //codes.find(substr1) != codes.end() && - has already checked that
					mem[i] = (codes[substr1] << 24) + (reg[substr2] << 20) + num;
	//          cout << mem[i] << " " << (mem[i] >> 24) << " " << ((mem[i] << 8) >> 28) << " " << ((mem[i] << 12)>> 12) << endl;
					i++;
				}
				else {
					string str = to_string(i + 1);
					str = "in line = " + str + " - incorrect register";
					char* c = new char[str.length()+1];
					strcpy(c, str.c_str());
					throw c;
				}
		}
	//==========================================================================================================
			else if (vec_type[codes[substr1]] == RR) {
				substr2 = process(str);
				substr3 = process(str);
				substr4 = process(str);
		//		  cout << "substr1 = " << substr1 << " substr2 = " << substr2 << " substr3 = " << substr3  << " substr4 = " << substr4 << endl;
		//------------------------------------------------------------------Convert string "Imm" to int "num"
				if (substr4[0] == '-') {
					for (unsigned int j = 1; j < substr4.length(); ++j){
						num2 = num2*10 - (substr4[j] - '0');
					}
						num = (unsigned int)(~(num2 - 1)) + C15 - 1;
				}
				else {
					for (unsigned int j = 0; j < substr4.length(); ++j){
						num = num*10 + substr4[j] - '0';
					}
				}		//----------------------------------------------------------
				if(reg.find(substr2) != reg.end() && reg.find(substr3) != reg.end() ) { //codes.find(substr1) != codes.end() && has already checked that
					mem[i] = (codes[substr1] << 24) + (reg[substr2] << 20) + (reg[substr3]<<16) + num;
		//			if (substr4[0] == '-') {
		//				cout << mem[i] << " " << (mem[i] >> 24) << " " << ((mem[i] << 8)>>28) << " " << ((mem[i] << 12) >> 28) << " " << (~((int)(((mem[i]<<16)>>16) - (C15-1))))+1 << endl;
		//			}
		//			else {
		//			    cout << mem[i] << " " << (mem[i] >> 24) << " " << ((mem[i] << 8)>>28) << " " << ((mem[i] << 12) >> 28) << " " << (int)((mem[i]<<16)>>16) << endl;
		//			}
					i++;
				}
				else {
					string str = to_string(i + 1);
					str = "in line = " + str + " - incorrect register(s)";
					char* c = new char[str.length()+1];
					strcpy(c, str.c_str());
					throw c;
				}
			}	//=================================================================================================================
			else if (vec_type[codes[substr1]] == RI) {
				substr2 = process(str);
				substr3 = process(str);
	//		    cout << "substr1 = " << substr1 << " substr2 = " << substr2 << " substr3 = " << substr3 << endl;
				//const unsigned int C19 = 524288;
				num2 = 0; //------------------------------------------------------------------Convert string "Imm" to int "num"
				if (substr3[0] == '-') {
					for (unsigned int j = 1; j < substr3.length(); ++j){
						num2 = num2*10 - (substr3[j] - '0');
					}
					num = (unsigned int)(~(num2 - 1)) + C19 - 1;
				}
				else {
					for (unsigned int j = 0; j < substr3.length(); ++j){
						num = num*10 + substr3[j] - '0';
					}
				}				//-------------------------------------------------------------------------------
				if(reg.find(substr2) != reg.end()) { //codes.find(substr1) != codes.end() &&  has already checked that
						mem[i] = (codes[substr1] << 24) + (reg[substr2] << 20) + num;
						if (substr3[0] == '-') {
	//	        cout << mem[i] << " " << (mem[i] >> 24) << " " << ((mem[i] << 8)>>28) << " " << (~((int)(((mem[i]<<12)>>12) - (C19-1))))+1 << endl;
						}
						else {
	//			    cout << mem[i] << " " << (mem[i] >> 24) << " " << ((mem[i] << 8)>>28) << " " << (int)((mem[i]<<12)>>12) << endl;
						}
						i++;
				}
				else {
					string str = to_string(i + 1);
					str = "in line = " + str + " - incorrect register";
					char* c = new char[str.length()+1];
					strcpy(c, str.c_str());
					throw c;
				}

			} //=========================================================================================
			else if (vec_type[codes[substr1]] == J) {
				substr2 = process(str);
	//		      cout << "substr1 = " << substr1  << " " << "substr2 = " << substr2 << endl;
				if (labels.find(substr2) != labels.end()) {
					num = labels[substr2];
				}
				else if (correct_address(substr2)) {num = stoul(substr2, nullptr, 0);} //for (unsigned int j = 0; j < substr3.length(); ++j){
				else {																																			 //num = num*10 + substr3[j] - '0';}
					string str = to_string(i + 1);
					str = "in line = " + str + " - incorrect address";
					char* c = new char[str.length()+1];
					strcpy(c, str.c_str());
					throw c;
				}
					mem[i] = (codes[substr1] << 24) + num; //if(codes.find(substr1) != codes.end()) has already checked that
	//	  	cout << mem[i] << " " << (mem[i] >> 24) << " " << ((mem[i] << 12) >> 12) << endl;
					i++;
			}
		}
	}
}
void CPU::MakeFbin() {
	ConstSize = 0;
  ExecFileName = "Exec" + FileName; // "ExecFileName.txt"
  ExecFileName.erase(ExecFileName.end()-4, ExecFileName.end()); //"ExecFileName"
  ExecFileName += ".fbin"; // "ExecFileName.bin"
//  ofstream fout (ExecFileName.c_str(), ios::trunc);
  ofstream bout(ExecFileName.c_str(), ios::binary|ios::out);
  char y[] = "ThisIsFUPM2Exec";
  bout.write((char*)&y, sizeof y); //write - "ThisIsFUPM2Exec"
  bout.write((char*)&n, sizeof n);
  bout.write((char*)&ConstSize, sizeof ConstSize);
  bout.write((char*)&DataSize, sizeof DataSize);
  bout.write((char*)&ExecPointer, sizeof ExecPointer);
  bout.write((char*)&StartStackPointer, sizeof StartStackPointer);
  unsigned int empty = 0;
  for (int i = 0; i < 119; i++) {
	bout.write((char*)&empty, sizeof empty);
  }
  for (unsigned int i = 0; i < n; i++) {
    bout.write((char*)&mem[i], sizeof mem[i]);
  }
  bout.close();
}

void CPU::MakeExecFile() {
	F.open(FileName.c_str(), ios::in);
  if (!F.is_open()) throw "Can't open file trying to create execution file";
	ReadLabels();
	ReadComands();
	F.close();
	MakeFbin();
}
void CPU::FillOfMemFromExe(ifstream &Fin) {
	unsigned int empty = 0;
	string str;
	char x[16];
    Fin.read((char*)&x, sizeof x);
    str = x;
    if (!(str == "ThisIsFUPM2Exec" )) throw "This is not FUPM2Exec file type";

	  Fin.read((char*)&n, sizeof n);
    Fin.read((char*)&ConstSize, sizeof ConstSize);
	  Fin.read((char*)&DataSize, sizeof DataSize);
	  Fin.read((char*)&ExecPointer, sizeof ExecPointer);
	  Fin.read((char*)&empty, sizeof empty); //StartStackPointer is constant value so shouldn't be read
      for (int i = 0; i < 119; i++) {
	    Fin.read((char*)&empty, sizeof empty);
      }
	  for (unsigned int i = 0; i < n; i++) Fin.read((char*)&mem[i], sizeof mem[i]); //Record comands

}
void CPU::Execution() {
	reg1[15] = ExecPointer;
  int syscallret = 0;
  for (unsigned int i = ExecPointer; i < (unsigned int)labels["end main"] + 1 && mem[i] != 0; ) { //halt is here
    //Protocol//--------------------------------------------------------------------------
/*
    cout << "This is before " << i << "-th comand execution //////////////////////" << endl;
    cout << endl;
  for (unsigned int j = C20 - 10; j < C20; j++) {
    cout << "mem[" << j << "] = " << mem[j];
    if ((int)j == reg1[14]) cout << "<--" << endl;
    else cout << endl;
  }
  for (unsigned int j = 0; j < RegistersSize; j++) {
    cout << "reg1[" << j << "] = " << reg1[j] << endl;
  }
  cout << endl;
*/            //-----------------------------------------------------------------------------
    switch(mem[i]>>24) {
      case SYSCALL: { //1
		if(Imm(mem[i]) == 100) cin >> reg1[reg_1(mem[i])];
		else if(Imm(mem[i]) == 101) { //SCANDOUBLE
			cin >> doub.d;
			reg1[reg_1(mem[i])] = (doub.a>>32);
			reg1[reg_1(mem[i])+1] = ((doub.a<<32)>>32);
		}
		else if(Imm(mem[i]) == 102) { cout << reg1[reg_1(mem[i])];}
		else if(Imm(mem[i]) == 103) { //PRINTDOUBLE
			doub.a = (((long long)reg1[reg_1(mem[i])])<<32) + reg1[reg_1(mem[i])+1];
			cout << doub.d << endl;
		}
		else if(Imm(mem[i]) == 104) reg1[reg_1(mem[i])] = getchar();
		else if(Imm(mem[i]) == 105) putchar(reg1[reg_1(mem[i])]);
		else if(Imm(mem[i]) == 0) syscallret = 1;
		else if(Imm(mem[i]) == 1) {
    	string NAME;
			cin >> NAME;
			int MODE;
			cin >> MODE;
			if (MODE == 0) { ifstream file; file.open(NAME, ios::in); }
			else if (MODE == 1) { ofstream file; file.open(NAME, ios::out); }
			else { fstream file; file.open(NAME, ios::in); }
		}
		i++; reg1[15]++;
		break;
	  }
      case ADD: { //2
		reg1[reg_1(mem[i])] += reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i]));
		i++; reg1[15]++;
	    break;
	  }
	  case ADDI: { //3
		reg1[reg_1(mem[i])] += uint_to_int(Imm(mem[i]));
		i++; reg1[15]++;
	    break;
	  }
	  case SUB: { //4
		reg1[reg_1(mem[i])] -= reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i]));
		i++; reg1[15]++;
	    break;
	  }
	  case SUBI: { //5
		reg1[reg_1(mem[i])] -= uint_to_int(Imm(mem[i]));
		i++; reg1[15]++;
	    break;
	  }
	  case MUL: { //6
			long long r1 = (long long)reg1[reg_1(mem[i])];
			long long r2 = (long long)reg1[reg_2(mem[i])] + (long long)uint_to_int_C15(Imm_RR(mem[i]));
			res.s = r1*r2;
			reg1[reg_1(mem[i])+1] = (int)((res.u>>32)&0x00000000FFFFFFFF);
			reg1[reg_1(mem[i])] = (int)(res.u&0x00000000FFFFFFFF);
			i++; reg1[15]++;
			break;
	  }
	  case MULI: { //7
			long long r1 = (long long)reg1[reg_1(mem[i])];
			long long imm = (long long)uint_to_int(Imm(mem[i]));
			res.s = r1*imm;
			reg1[reg_1(mem[i])+1] = (int)((res.u>>32)&0x00000000FFFFFFFF);
			reg1[reg_1(mem[i])] = (int)(res.u&0x00000000FFFFFFFF);
			i++; reg1[15]++;
			break;
	  }
	  case DIV: { //8
		int r1 = reg1[reg_1(mem[i])];
		int r2 = reg1[reg_1(mem[i]) + 1];
		int r10 = reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i]));
		if (r10 == 0) throw "division by zero";
		if ((((((long long)r1 + ((long long)r2 << 31))/(long long)r10)>>32)<<32) != 0) {
		  cout << "Division is incorrect (division by 0 or overflow) (line " << i << ")\n";
		}
		else {
		  reg1[reg_1(mem[i])] = (int)(((((long long)r1 + ((long long)r2 << 31))/(long long)r10)<<32)>>32);
		  reg1[reg_1(mem[i]) + 1] = (int)(((((long long)r1 + ((long long)r2 << 31))%(long long)r10)<<32)>>32);
		}
		i++; reg1[15]++;
		break;
	  }
	  case DIVI: { //9
		int r1 = reg1[reg_1(mem[i])];
		int r2 = reg1[reg_1(mem[i]) + 1];
		int r10 = uint_to_int(Imm(mem[i]));
		if (r10 == 0) throw "division by zero";
		if ((((((long long)r1 + ((long long)r2 << 31))/(long long)r10)>>32)<<32) != 0) {
		  cout << "Division is incorrect (division by 0 or overflow) (line " << i << ")\n";
		}
		else {
		  reg1[reg_1(mem[i])] = (int)(((((long long)r1 + ((long long)r2 << 31))/(long long)r10)<<32)>>32);
		  reg1[reg_1(mem[i]) + 1] = (int)(((((long long)r1 + ((long long)r2 << 31))%(long long)r10)<<32)>>32);
		}
		i++; reg1[15]++;
		break;
	  }
	  case LC: { //12
			reg1[reg_1(mem[i])] = uint_to_int(Imm(mem[i]));
			i++; reg1[15]++;
			break;
	  }
	  case SHL: { //13
			int r2 = reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i]));
			if (r2 >= 0) {
			  reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] << r2;
			}
			else {
		      reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] >> (-r2);
		    }
		    i++; reg1[15]++;
			break;
	  }
	  case SHLI: { //14
		int imm = uint_to_int(Imm(mem[i]));
		if (imm >= 0) {
		  reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] << imm;
		}
		else {
	      reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] >> (-imm);
	    }
	    i++; reg1[15]++;
		break;
	  }
	  case SHR: { //15
		int r2 = reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i]));
		if (r2 >= 0) {
		  reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] >> r2;
		}
		else {
	      reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] << (-r2);
	    }
	    i++; reg1[15]++;
		break;
	  }
	  case SHRI: { //16
			int imm = uint_to_int(Imm(mem[i]));
			if (imm >= 0) {
			  reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] >> imm;
			}
			else {
		      reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] << (-imm);
		    }
		    i++; reg1[15]++;
			break;
	  }
	  case AND: { //17
	    reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] & (reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i])));
	    i++; reg1[15]++;
	    break;
	  }
	  case ANDI: { //18
			reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] & uint_to_int(Imm(mem[i]));
			i++; reg1[15]++;
		  break;
	  }
	  case OR: { //19
	    reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] | (reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i])));
	    i++; reg1[15]++;
	    break;
	  }
	  case ORI: { //20
			reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] | uint_to_int(Imm(mem[i]));
			i++; reg1[15]++;
	    break;
	  }
	  case XOR: { //21
	    reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] ^ (reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i])));
	    i++; reg1[15]++;
	    break;
	  }
	  case XORI: { //22
			reg1[reg_1(mem[i])] = reg1[reg_1(mem[i])] ^ uint_to_int(Imm(mem[i]));
	    i++; reg1[15]++;
	    break;
	  }
	  case NOT: { //23
			reg1[reg_1(mem[i])] = ~reg1[reg_1(mem[i])];
			i++; reg1[15]++;
	    break;
	  }
	  case MOV: { //24
			reg1[reg_1(mem[i])] = reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i]));
			i++; reg1[15]++;
	    break;
	  }
	  case ADDD: { //32
			doub.a = (((long long)reg1[reg_1(mem[i])])<<32) + (long long)reg1[reg_1(mem[i])+1];
			doub2.a = (((long long)reg1[reg_2(mem[i])])<<32) + (long long)reg1[reg_2(mem[i])+1];
		  doub.d += (doub2.d + (double)uint_to_int_C15(Imm_RR(mem[i])));
		  reg1[reg_1(mem[i])] = (doub.a>>32)&0x00000000FFFFFFFF;
			reg1[reg_1(mem[i])+1] = doub.a&0x00000000FFFFFFFF;
			i++; reg1[15]++;
			break;
	  }
	  case SUBD: { //33
		doub.a = (((long long)reg1[reg_1(mem[i])])<<32) + (long long)reg1[reg_1(mem[i])+1];
		doub2.a = (((long long)reg1[reg_2(mem[i])])<<32) + (long long)reg1[reg_2(mem[i])+1];
		doub.d -= (doub2.d + (double)uint_to_int_C15(Imm_RR(mem[i])));
	  reg1[reg_1(mem[i])] = (doub.a>>32)&0x00000000FFFFFFFF;
		reg1[reg_1(mem[i])+1] = doub.a&0x00000000FFFFFFFF;
		i++; reg1[15]++;
		break;
	  }
	  case MULD: { //34
		doub.a = (((long long)reg1[reg_1(mem[i])])<<32) + (long long)reg1[reg_1(mem[i])+1];
		doub2.a = (((long long)reg1[reg_2(mem[i])])<<32) + (long long)reg1[reg_2(mem[i])+1];
		doub.d *= (doub2.d + (double)uint_to_int_C15(Imm_RR(mem[i])));
	  reg1[reg_1(mem[i])] = (doub.a>>32)&0x00000000FFFFFFFF;
		reg1[reg_1(mem[i])+1] = doub.a&0x00000000FFFFFFFF;
		i++; reg1[15]++;
		break;
	  }
	  case DIVD: { //35
		doub.a = (((long long)reg1[reg_1(mem[i])])<<32) + (long long)reg1[reg_1(mem[i])+1];
		doub2.a = (((long long)reg1[reg_2(mem[i])])<<32) + (long long)reg1[reg_2(mem[i])+1];
		doub2.d += (double)uint_to_int_C15(Imm_RR(mem[i]));
		if (doub2.d == 0) throw "division by zero";
		doub.d /= doub2.d;
	  reg1[reg_1(mem[i])] = (doub.a>>32)&0x00000000FFFFFFFF;
		reg1[reg_1(mem[i])+1] = doub.a&0x00000000FFFFFFFF;
		i++; reg1[15]++;
		break;
	  }

	  case ITOD: { //36
			doub.d = (double)reg1[reg_2(mem[i])];
			reg1[reg_1(mem[i])] = (doub.a>>32)&0x00000000FFFFFFFF;
			reg1[reg_1(mem[i])+1] = doub.a&0x00000000FFFFFFFF;
			i++; reg1[15]++;
			break;
	  }
	  case DTOI: { //37
			intuint r2, r3;
			r2.s = (long long)reg1[reg_2(mem[i])];
			r3.s = (long long)reg1[reg_2(mem[i])+1];
			doub.u = (r2.u<<32) | r3.u;
			if(doub.d < -(double)(((unsigned long long)1)<<32)  || (double)(((unsigned long long)1)<<32) < doub.d) cout << "can't convert double to int (overflow)" << endl;
			reg1[reg_1(mem[i])] = (int)doub.d;
			i++; reg1[15]++;
			break;
	  }
    case PUSH: { //38
	    mem[reg1[14]] = int_to_uint(reg1[reg_1(mem[i])]) + Imm(mem[i]);
	    reg1[14]--;
	    i++; reg1[15]++;
      break;
	  }
	  case POP: { //39
	    reg1[14]++;
	    reg1[reg_1(mem[i])] = uint_to_int(mem[reg1[14]]) + uint_to_int(Imm(mem[i]));
	    i++; reg1[15]++;
	    break;
	  }
	  case CALL: { //40
		mem[reg1[14]] = i + 1;
		reg1[14]--;
        reg1[15] = uint_to_int(Imm_RR(mem[i])) + reg1[reg_2(mem[i])];
        i = int_to_uint(reg1[reg_2(mem[i])]) + Imm_RR(mem[i]);
        reg1[reg_1(mem[i])] = i;
        break;
	  }
	  case CALLI: { //41
		mem[reg1[14]] = i + 1;
		reg1[14]--;
		reg1[15] = uint_to_int(Imm(mem[i]));
		i = Imm(mem[i]);
		break;
	  }
	  case RET: { //42
		reg1[14] = reg1[14] + uint_to_int(Imm(mem[i])) + 1;
		reg1[15] = uint_to_int(mem[reg1[14]]);
		i = mem[reg1[14]];
		break;
	  }
	  case CMP: { //43
		if( reg1[((mem[i]<<8)>>28)] > reg1[((mem[i]<<12)>>28)] ) reg1[16] = 1;
		else if( reg1[((mem[i]<<8)>>28)] == reg1[((mem[i]<<12)>>28)] ) reg1[16] = 0;
		else reg1[16] = -1; //16-ый регистр - это регистр флагов
		i++; reg1[15]++;
		break;
	  }
	  case CMPI: { //44
		if( reg1[((mem[i]<<8)>>28)] > uint_to_int(((mem[i]<<12)>>12)) ) reg1[16] = 1;
		else if( reg1[((mem[i]<<8)>>28)] == uint_to_int(((mem[i]<<12)>>12)) ) reg1[16] = 0;
		else reg1[16] = -1;
		i++; reg1[15]++;
		break;
	  }
	  case CMPD: { //45
			doub.a = (((long long)reg1[reg_1(mem[i])])<<32) + reg1[reg_1(mem[i])+1];
			doub2.a = (((long long)reg1[reg_2(mem[i])])<<32) + reg1[reg_2(mem[i])+1];
			cout << "doub =" << doub.d << endl;
			cout << "doub2 =" << doub2.d << endl;
			if( doub.d > doub2.d ) reg1[16] = 1;
			else if( doub.d < doub2.d ) reg1[16] = -1;
			else reg1[16] = 0;
			cout << "flag = " << reg1[16] << endl;
			i++; reg1[15]++;
	    break;
	  }
	  case JMP: { //46
	    reg1[15] = Imm(mem[i]);
	    i = Imm(mem[i]);
	    break;
	  }
	  case JNE: { //47
			if (reg1[16] != 0) {
				reg1[15] = Imm(mem[i]);
				i = Imm(mem[i]);
			}
			else {i++; reg1[15]++;}
			break;
		  }
	  case JEQ: { //48
			if (reg1[16] == 0) {
				reg1[15] = Imm(mem[i]);
				i = Imm(mem[i]);
			}
			else {i++; reg1[15]++;}
			break;
	  }
	  case JLE: { //49
			if (reg1[16] != 1) {
				reg1[15] = Imm(mem[i]);
				i = Imm(mem[i]);
			}
			else {i++; reg1[15]++;}
			break;
	  }
	  case JL: { //50
			if (reg1[16] == -1) {
				reg1[15] = Imm(mem[i]);
				i = Imm(mem[i]);
			}
			else {i++; reg1[15]++;}
			break;
	  }
	  case JGE: { //51
			if (reg1[16] != -1) {
				reg1[15] = Imm(mem[i]);
				i = Imm(mem[i]);
			}
			else {i++; reg1[15]++;}
			break;
	  }
	  case JG: { //52
			if (reg1[16] == 1) {
				reg1[15] = Imm(mem[i]) - 1;
				i = Imm(mem[i]);
			}
			else {i++; reg1[15]++;}
			break;
	  }
	  case LOAD: { //64
		reg1[reg_1(mem[i])] = uint_to_int(mem[Imm(mem[i])]);
		i++; reg1[15]++;
		break;
	  }
	  case STORE: { //65
		mem[ Imm(mem[i]) ] = int_to_uint(reg1[reg_1(mem[i])]);
		i++; reg1[15]++;
		break;
	  }
	  case LOAD2: { //66
		reg1[reg_1(mem[i])] = uint_to_int(mem[Imm(mem[i])]);
		reg1[reg_1(mem[i])+1] = uint_to_int(mem[Imm(mem[i])+1]);
		i++; reg1[15]++;
		break;
	  }
	  case STORE2: { //67
			mem[Imm(mem[i])] = int_to_uint(reg1[reg_1(mem[i])]);
			mem[Imm(mem[i])+1] = int_to_uint(reg1[reg_1(mem[i])+1]);
			i++; reg1[15]++;
			break;
	  }
	  case LOADR: { //68
			long long local_address = (long long)(reg1[reg_2(mem[i])] + uint_to_int_C15(Imm_RR(mem[i])));
			if (local_address < 0 || local_address >= (long long)(C20)) throw "unaddressable access";
			reg1[reg_1(mem[i])] = uint_to_int(mem[ (unsigned int)local_address ]);
			i++; reg1[15]++;
			break;
	  }
	  case STORER: { //69
		mem[ Imm_RR(mem[i]) + int_to_uint(reg1[reg_2(mem[i])]) ] = int_to_uint(reg1[reg_1(mem[i])]);
		i++; reg1[15]++;
		break;
	  }
	  case LOADR2: { //70
		reg1[reg_1(mem[i])] = uint_to_int(mem[ reg1[reg_2(mem[i])] + Imm_RR(mem[i]) ]);
		reg1[reg_1(mem[i])+1] = uint_to_int(mem[ reg1[reg_2(mem[i])] + Imm_RR(mem[i]) + 1 ]);
		i++; reg1[15]++;
		break;
	  }
	  case STORER2: { //71
		mem[ Imm_RR(mem[i]) + int_to_uint(reg1[reg_2(mem[i])]) ] = int_to_uint(reg1[reg_1(mem[i])]);
		mem[ Imm_RR(mem[i]) + int_to_uint(reg1[reg_2(mem[i])]) + 1] = int_to_uint(reg1[reg_1(mem[i])+1]);
		i++; reg1[15]++;
		break;
	  }
    }

    if (syscallret == 1) break;
  }
}
void CPU::RunExecFile() {
  ifstream Fin(ExecFileName.c_str(), ios::binary|ios::in); //--------------------------------------------------------------------------
  											//Fill of mem[i] from ExecFile
  if (!Fin.is_open()) throw "Can't open file trying to run execution file";
	FillOfMemFromExe(Fin);
  Fin.close();
	Execution();
}
void CPU::Disassembler() {
	AssemblyFileName = ExecFileName;
	AssemblyFileName.erase(0, 4);
  AssemblyFileName = "Assembly" + AssemblyFileName; // "AssemblyFileName.bin"
  AssemblyFileName.erase(AssemblyFileName.end()-4, AssemblyFileName.end()); // "AssemblyFileName"
  AssemblyFileName += "txt";

	ifstream Fin(ExecFileName.c_str(), ios::binary|ios::in); //--------------------------------------------------------------------------
	unsigned int empty = 0;											//Fill of mem[i] from ExecFile
	if (!Fin.is_open()) throw "Can't open ExecFile";

	string str;
	char x[16];
	Fin.read((char*)&x, sizeof x);
	str = x;
	if (!(str == "ThisIsFUPM2Exec" )) throw "This is not FUPM2Exec file type";
  Fin.read((char*)&n, sizeof n);
  Fin.read((char*)&ConstSize, sizeof ConstSize);
  Fin.read((char*)&DataSize, sizeof DataSize);
  Fin.read((char*)&ExecPointer, sizeof ExecPointer);
  Fin.read((char*)&empty, sizeof empty); //StartStackPointer is constant value so shouldn't be read
  for (int i = 0; i < 119; i++) {
	Fin.read((char*)&empty, sizeof empty);
  }
  for (unsigned int i = 0; i < n; i++) Fin.read((char*)&mem[i], sizeof mem[i]); //Record comands

	Fin.close();

	ofstream AssemblyOut (AssemblyFileName.c_str(), ios::trunc);
	for (unsigned int i = 0; i < DataSize; i++) {
		var.push_back("var_" + to_string(i));
	}

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int varcount = 0;
	string lname;
	lname = "l_";
	while (i < n) {
		if (vec_type[com(mem[i])] == J && l.find(Imm(mem[i])) == l.end()) {
		  lname += to_string(j);
			l[ Imm(mem[i]) ] = lname;
			lname = "l_";
			j++;
		}
		if (vec_type[com(mem[i])] == RM && var_s.find(Imm(mem[i])) == var_s.end()) {
			var_s[ Imm(mem[i]) ] = var[ varcount ];
			varcount++;
		}
		i++;
	}

	i = 0;
	bool flagmain = 0;
	while (i < n) {
	  if (i == ExecPointer) { AssemblyOut << "main:\n"; flagmain = 1;}
	  if (l.find(i) != l.end() ) {
//	    if (flagmain == 1) AssemblyOut << "end main" << endl;
	    AssemblyOut << l[ i ] << ":" << endl;
	  }
	  if (vec_type[com(mem[i])] == RM) {
			AssemblyOut << comands[com(mem[i])] << " " << r[reg_1(mem[i])] << " " << var_s[Imm(mem[i])] << endl;
	  }
	  else if (vec_type[com(mem[i])] == RR) {
			AssemblyOut << comands[com(mem[i])] << " " << r[reg_1(mem[i])] << " " << r[reg_2(mem[i])] << " " << Imm_RR(mem[i]) << endl;
	  }
	  else if (vec_type[com(mem[i])] == RI) {
    	AssemblyOut << comands[com(mem[i])] << " " << r[reg_1(mem[i])] << " " << Imm(mem[i]) << endl;
	  }
	  else if (vec_type[com(mem[i])] == J) {
			AssemblyOut << comands[com(mem[i])] << " " << l[ Imm(mem[i]) ] << endl;
	  }
		if (i == n - 1 && DataSize != 0) {
			for (unsigned int j = 0; j < DataSize; j++) {
				AssemblyOut << var[j] << ":" << endl;
			}
		}
	  if (i == n - 1 && flagmain == 1) AssemblyOut << "end main" << endl;
	  i++;
    }
    AssemblyOut.close();
}

int main(int argc, char * argv[])
{
	CPU objectA;
//	objectA.FileName = "Fops.txt";
	objectA.FileName = argv[1];
	try {
		objectA.MakeExecFile();
	}
	catch (const char* c) {
		cout << "error1: " << c << endl;
	}
	try {
		objectA.RunExecFile();
	}
	catch (const char* c) {
		cout << "error2: " << c << endl;
	}
	try {
		objectA.Disassembler();
	}
	catch (const char* c) {
		cout << "error3: " << c << endl;
	}
	return 0;
}
