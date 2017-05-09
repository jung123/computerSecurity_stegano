#include <iostream>
#include <fstream>
#include <cstdarg>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
// 트루컬러 bmp 파일이여서 팔레트가 존재하지 않는다 ! 
using namespace std;

class ReadBmp{
	private :
	uint32_t i;
	ifstream ifs;
	vector<uint8_t> bmpHeader;
	vector<uint8_t> bmpBody;
	streampos bodyStartPos;
	vector<uint8_t> result;
	//
	uint32_t filesize;
	uint32_t bitmap_data_off_set;
	uint32_t bitmap_header_size;
	uint32_t width;
	uint32_t height;
	uint32_t bitPerPixel;
	//
	public : 
	~ReadBmp(){
		if(ifs.is_open()) this->ifs.close();
	}
	//
	bool open(string str);
	bool headerParsing();
	void toInt(vector<uint8_t>& vec, uint32_t& to);
	void readBody();
	void handleBit(char *buffer, int index, uint8_t& tmp, uint32_t& j);
};

int main(){
	ReadBmp openBmp;
	if(!openBmp.open("alice_stego.bmp")) cout << "Error : open()" << endl;
	openBmp.headerParsing();
	openBmp.readBody();
	
	return 0;	
}
bool ReadBmp::open(string str){
	this->ifs.open(str,ifstream::binary);
	if(!this->ifs.is_open()) return false;
	return true;
}
bool ReadBmp::headerParsing(){
	char tmp;
	uint32_t i = 0;
	vector<uint8_t> vec;
	vec.reserve(4);
	uint32_t headerSize;
	streampos start = ifs.tellg();
	while(true){
		this->ifs >> tmp;
		i++;
		if((headerSize != 0)&&( i == headerSize)) break;
		
		// filesize
		if( i>=3 && i<=6 ){
			vec.push_back(tmp);
			if(i==6) toInt(vec, this->filesize);
		} 
		// bitmap data off set
		if( i==11 ) {
			this->bitmap_data_off_set = ((tmp >> 4) * 16) + ((tmp << 28) >> 28);
			headerSize = this->bitmap_data_off_set;
		}
		// bitmap header size
		if( i>=15 && i<=18 ){
			vec.push_back(tmp);
			if(i==18) toInt(vec, this->bitmap_header_size);
		}
		// width
		if( i>=19 && i<=22 ){
			vec.push_back(tmp);
			if(i==22) toInt(vec, this->width);
		}
		// height
		if( i>=23 && i<=26 ){
			vec.push_back(tmp);
			if(i==26) toInt(vec, this->height);
		}
		// bits per pixel
		if(( i >= 29) && ( i<= 30)){
			vec.push_back(tmp);
			if(i == 30) toInt(vec, this->bitPerPixel);	
		}
	/*	// magic
		if(( i >= 1) && ( i<= 2)) vec.pop_back();
		// planes
		if(( i >= 27) && ( i<= 28)) vec.pop_back();
		// compression
		if(( i >= 31) && ( i<= 34)) vec.pop_back();
		// bitmap data size
		if(( i >= 35) && ( i<= 38)) vec.pop_back();
		// hresolution
		if(( i >= 39) && ( i<= 42)) vec.pop_back();
		// vresolution
		if(( i >= 43) && ( i<= 46)) vec.pop_back();
		// colors
		if(( i >= 47) && ( i<= 50)) vec.pop_back();
		// important colors
		if(( i >= 51) && ( i<= 54)) vec.pop_back();
		// palette 
	*/	
	}
	// 
	this->i = ++i;
	this->bodyStartPos = ifs.tellg();
	this->bmpBody.reserve(this->filesize - bitmap_header_size);
	
	cout << "-------------- Header Information ----------------" << endl;
	cout << "I : 0x" << std::hex << i << std::dec << endl;
	cout << "Bitmap Header size : " << this->bitmap_header_size << endl;
	cout << "bitmap data off set : 0x" << std::hex << this->bitmap_data_off_set << std::dec << endl;
	cout << "file size : " << this->filesize << endl;
	cout << "width : " << this->width << endl;
	cout << "height : " << this->height << endl;
	cout << "bitPerPixel : " << this->bitPerPixel << endl;
	cout << "True color bmp : width * height * 3byte(rgb) = " << (3*this->height*this->width) << endl;
	cout << "--------------------------------------------------" << endl;
}
void ReadBmp::toInt(vector<uint8_t>& vec, uint32_t& to){	
	to = 0;
	uint32_t i;
	for(i = vec.size();i>1;i--){
		to = to | static_cast<uint32_t>(vec.back());
		to = to << 8;
		vec.pop_back();
	}
	to = to | static_cast<uint32_t>(vec.back());
	vec.pop_back();
}

void ReadBmp::readBody(){
	this->result.clear();
	cout << "readBody() Start !! => result.bmp" << endl;
	this->ifs.seekg(this->bodyStartPos, ios_base::beg);
	//
	this->ifs.seekg(this->bodyStartPos, ios_base::end);
	cout << "read end pos : " << this->ifs.tellg() << endl;
	//
	uint32_t imageSize = this->height * this->width * 3;
	uint32_t lineByteNum = imageSize / this->height;
	uint32_t j;
	uint8_t tmp = 0;
	// for test code
	cout << "imageByteNum : " << imageSize << endl;
	cout << "lineByteNum : " << lineByteNum << endl;
	cout << "width * 3 = " << this->width * 3 << endl;
	cout << "lineByteNum * height = " << lineByteNum * this->height << endl;
	cout << "lineByteNum % 8 : " << lineByteNum % 8 << endl;
	// RGB Parsing !
	char *buffer = new char[lineByteNum]{0,};
	for(uint32_t i = 0; i < this->height; i++) {
		//
		this->ifs.seekg(-(static_cast<int>(lineByteNum * (i+1))), ios_base::end);
		this->ifs.read(buffer, lineByteNum);
		if(this->ifs.bad()){
			cout << "ifs is bad() !!" << endl;
			break;
		}else if(this->ifs.fail()){
			cout << "ifs is fail() !!" << endl;
			break;
		}else if(!this->ifs.good()){
			cout << "ifs is not Good() !!" << endl;
			break;
		}
		//
		for(j = 1; j <= lineByteNum; j) {
			this->handleBit(buffer, j-1, tmp, j);
		}
		//
	}
	//
	ofstream ofs("result.bmp");
	if(!ofs.is_open()) {
		cout << "ofs not open !" << endl;
		return;
	}
	for(auto& t : this->result) ofs << t;
	ofs.close();
	//
	delete[] buffer;
	cout << "---------------------------------------" << endl;
}

void ReadBmp::handleBit(char *buffer, int index, uint8_t& tmp, uint32_t& j){
	// index = j-1;
	uint8_t b = static_cast<uint8_t>(buffer[index]);
	uint8_t g = static_cast<uint8_t>(buffer[index+1]);
	uint8_t r = static_cast<uint8_t>(buffer[index+2]);
	//
	tmp = tmp << 1;
	tmp = tmp | (r & 1);
	if(j%8 == 0){
		this->result.push_back(tmp);
		tmp = 0;
	}
	j++;
	//
	tmp = tmp << 1;
	tmp = tmp | (g & 1);
	if(j%8 == 0){
		this->result.push_back(tmp);
		tmp = 0;
	}
	j++;
	//
	tmp = tmp << 1;
	tmp = tmp | (b & 1);
	if(j%8 == 0){
		this->result.push_back(tmp);
		tmp = 0;
	}
	j++;
	
}



