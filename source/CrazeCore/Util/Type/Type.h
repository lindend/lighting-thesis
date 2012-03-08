#pragma once
#ifndef CRAZE__TYPE__H__
#define CRAZE__TYPE__H__

namespace Craze
{

	/**
	Class to use for describing types.
	*/
	class Type
	{
	public:
		/**
		@return The unique identifier of the type.
		*/
		virtual unsigned long GetId() const { return m_Id; }
		/**
		Sets the unique type identifier for the type.
		*/
		virtual void SetId(unsigned long id) { m_Id = id; }

		virtual ~Type() { }

	protected:
		/**
		The identifier for the type.
		*/
		unsigned long m_Id;
	};

}

#endif /*CRAZE__TYPE__H__*/