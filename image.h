#pragma once

#ifndef WWIMAGE_H
#define WWIMAGE_H

#include <data_type.h>
//#include <memory>

//using std::shared_ptr;
class Image
{
public:
	Image();
	Image(const char* filename);
	~Image();

	inline uchar* ptr() const{
		return m_data;
	}
	inline uchar* ptr(const int& row) const{
		return m_data + row * m_stepsize;
	}
	inline uchar* ptr(const int& row, const int& col){
  		return m_data + row * m_stepsize + col * m_channel;
	}

	uint m_width;
	uint m_height;
	uint m_stepsize;
	uchar m_bpp;
	uchar m_channel;
	uchar* m_data;
};

template <class T> class Image_ : public Image
{
public:
	Image_(const uint& _width, const uint& _height, const uchar& _channel,const int& _value = -1)
	{
		m_width = _width;
		m_height = _height;
		m_bpp = sizeof(T) * _channel * 8;
		m_channel = _channel;
		m_stepsize = m_width * m_channel;
		m_data = new uchar[m_width * m_height * m_bpp / 8];

		if (_value != -1){
			T* temp = (T*)m_data;
			for (uint i = 0; i < m_stepsize * m_height; i++)
			{
				*temp = _value;
				temp++;
			}
		}
	}
  
  	inline T* ptr() const{
		return (T*)m_data;
	}

	inline T* ptr(const int& row) const{
		return T*(m_data) + row * m_stepsize;
	}

	inline T* ptr(const int& row, const int& col){
		return T*(m_data) + row * m_stepsize + col* m_channel;
	}

};

#endif
