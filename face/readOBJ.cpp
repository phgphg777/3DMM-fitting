#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <string>
using namespace std;

template<typename value, typename index>
bool readOBJ_general(const std::string& filename,
	vector< vector<value> > & V, vector< vector<value> > & N, vector< vector<value> > & TC,
	vector< vector<index> > & F, vector< vector<index> > & FN, vector< vector<index> > & FTC);



void readOBJ(const std::string& filename,
	vector<array<float,3>>& V, vector<array<int,3>>& I, 
	vector<array<float,3>>& N, vector<array<int,3>>& NI)
{
	std::string ext_name(filename);
	ext_name.erase(0, ext_name.rfind('.')+1);
	if (ext_name != "obj") 
		throw std::runtime_error("This file is not .obj file..");

	std::vector< std::vector<float> > _V, _N, _T;
	std::vector< std::vector<int> > _I, _NI, _TI;
	if ( !readOBJ_general(filename, _V, _N, _T, _I, _NI, _TI) )
		throw std::runtime_error("OBJ file loading failed..");
	
	auto copy = [&](const auto& src, auto& trg, int inner_size) 
	{
		trg.resize(src.size());
		for (int i = 0; i < (int)src.size(); ++i) 
			for (int j = 0; j < inner_size; ++j) 
				trg[i][j] = src[i][j];
	};

	copy(_V, V, 3);
	copy(_I, I, 3);
	copy(_N, N, 3);
	copy(_NI, NI, 3);
}









#define LINE_MAX 2048
template<typename value, typename index>
bool readOBJ_general(const std::string& filename,
	vector< vector<value> > & V, vector< vector<value> > & N, vector< vector<value> > & TC,
	vector< vector<index> > & F, vector< vector<index> > & FN, vector< vector<index> > & FTC)
{
	FILE * obj_file = fopen(filename.c_str(), "r");
	if(NULL==obj_file)
	{
		fprintf(stderr,"IOError: %s could not be opened...\n", filename.c_str());
		return false;
	}
	V.clear();N.clear();TC.clear();
	F.clear();FN.clear();FTC.clear();

	string v("v");
	string vn("vn");
	string vt("vt");
	string f("f");
	string tic_tac_toe("#");

	char line[LINE_MAX];
	char type[LINE_MAX];
	int line_no = 1;
	while( fgets(line, LINE_MAX, obj_file) != NULL ) 
	{
		if( sscanf(line, "%s",type) != 1 )	//Áï, empty lineÀÌ¸é
		{
			line_no++;
			continue;
		}
		
		char * l = line + strlen(type);	// Get pointer to rest of line right after type
		
		if(type == v)
		{
			double x[4];
			int count = sscanf(l,"%lf %lf %lf %lf\n",&x[0],&x[1],&x[2],&x[3]);
			if(count != 3 && count != 4) {
				fprintf(stderr, "Error: readOBJ() vertex on line %d should have 3 or 4 coordinates", line_no);
				fclose(obj_file);
				return false;
			}
			vector<value> vertex(count);
			for(int i = 0;i<count;i++) 
				vertex[i] = (value) x[i];
			
			V.push_back(vertex);
		}

		else if(type == vn)
		{
			double x[3];
			int count = sscanf(l,"%lf %lf %lf\n",&x[0],&x[1],&x[2]);
			if(count != 3) {
				fprintf(stderr, "Error: readOBJ() normal on line %d should have 3 coordinates", line_no);
				fclose(obj_file);
				return false;
			}
			vector<value> normal(count);
			for(int i = 0;i<count;i++)
				normal[i] = (value) x[i];

			N.push_back(normal);
		}

		else if(type == vt)
		{
			double x[3];
			int count = sscanf(l,"%lf %lf %lf\n",&x[0],&x[1],&x[2]);
			if(count != 2 && count != 3) {
				fprintf(stderr, "Error: readOBJ() vertex on line %d should have 2 or 3 coordinates (%d)", line_no,count);
				fclose(obj_file);
				return false;
			}
			vector<value> tex(count);
			for(int i = 0;i<count;i++)
				tex[i] = (value) x[i];
			
			TC.push_back(tex);
		}

		else if(type == f)
		{
			vector<index> f;
			vector<index> fn;
			vector<index> ftc;
			
			// Read each "word" after type
			char word[LINE_MAX];
			int offset;
			while(  sscanf(l, "%s%n", word, &offset) == 1  )
			{
				l += offset;

				unsigned int i,it,in;
				if(sscanf(word,"%u/%u/%u",&i,&it,&in) == 3) {
					f.push_back(i-1);
					ftc.push_back(it-1);
					fn.push_back(in-1);
				} else if(sscanf(word,"%u/%u",&i,&it) == 2) {
					f.push_back(i-1);
					ftc.push_back(it-1);
				} else if(sscanf(word,"%u//%u",&i,&in) == 2) {
					f.push_back(i-1);
					fn.push_back(in-1);
				} else if(sscanf(word,"%u",&i) == 1) {
					f.push_back(i-1);
				} else {
					fprintf(stderr, "Error: readOBJ() face on line %d has invalid element format\n", line_no);
					fclose(obj_file);
					return false;
				}
			}
			
			if(f.size()!=3)
			{
				fprintf(stderr, "Error: Our face must be triangle!! But line %d has not three indices\n", line_no);
				fclose(obj_file);
				return false;
			}
			bool tmp = (fn.size() == 0 && ftc.size() == 0)		  ||  (fn.size() == f.size() && ftc.size() == 0)   ||
					   (fn.size() == 0 && ftc.size() == f.size()) ||  (fn.size() == f.size() && ftc.size() == f.size()) ;
			if(tmp==false)
			{
				fprintf(stderr, "Error: readOBJ() face on line %d has invalid format\n", line_no);
				fclose(obj_file);
				return false;
			}
			F.push_back(f);
			FN.push_back(fn);
			FTC.push_back(ftc);
		}
		
		else if(  strlen(type) >= 1 && (type[0] == '#' ||  type[0] == 'g'  || type[0] == 's'  ||
				  strcmp("usemtl",type)==0 ||  strcmp("mtllib",type)==0)  )
		{
			//ignore comments or other shit
		} 
		else 
		{
			//ignore any other lines
			fprintf(stderr, "Warning: readOBJ() ignored non-comment line %d:\n  %s", line_no, line);
		}
    
		line_no++;
	}

	fclose(obj_file);

	return true;
}
