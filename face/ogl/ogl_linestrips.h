#pragma once
#include <set>
#include <memory>
#include "ogl_core.h"
namespace hmp {
	class Hair;
};

class LineStrips : public OpenGLObject
{
	int numStrip;
	int sizeStrip;		//It denote the number of vertices per each strip..

	int* buffer1;		//These are buffers for glMultiDrawArrays..
	int* buffer2;
	void setBuffer();

	std::set<int> selected_strips;
	bool selected_only = false;

	VBO position;
	VBO color;

	LineStrips(){}
	virtual uint organize_vao() const;
	virtual void put(uint vao, const ogl::mat4& VP) const;

public:  
	static std::shared_ptr<LineStrips> create(const std::string& filename);
	static std::shared_ptr<LineStrips> create(const hmp::Hair& hair);
	virtual ~LineStrips();
	void chageDrawMode() { selected_only = !selected_only; }
	std::set<int>& selected_strip_set() { return selected_strips; }	
};

