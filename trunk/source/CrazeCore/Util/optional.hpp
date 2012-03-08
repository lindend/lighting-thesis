#ifndef __OPTIONAL__H__INCLUDED__
#define __OPTIONAL__H__INCLUDED__

template <typename _T> class optional
{
public:
	optional()
	{
		m_Valid = false;
	}
	optional(const _T& obj)
	{
		m_Obj = obj; 
		m_Valid = true;
	}

	bool Valid()
	{
		return m_Valid;
	}

	void Reset()
	{
		m_Valid = false;
	}

	_T& Get()
	{
		return m_Obj;
	}

	_T& operator= (const _T& obj)
	{
		m_Obj = obj; 
		m_Valid = true; 
		return m_Obj;
	}
protected:
	_T m_Obj;
	bool m_Valid;
};


#endif /*__OPTIONAL__H__INCLUDED__*/