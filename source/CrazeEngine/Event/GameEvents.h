#pragma once
#include "CrazeEngine.h"
#include "EngineExport.h"

#include "IEvent.h"

namespace Craze
{
	class CRAZEENGINE_EXP LoadLevelEvent : public IEvent
	{
	public:
		LoadLevelEvent(std::string fileName) { m_FileName = fileName; }

		static const EventType Type;
		virtual EventType VGetType() const { return Type; }

		const std::string& GetFileName() const { return m_FileName; }

	protected:
		std::string m_FileName;
	};

	class CRAZEENGINE_EXP ExitGameEvent : public IEvent
	{
	public:
		static const EventType Type;
		virtual EventType VGetType() const { return Type; }
	};

	class CRAZEENGINE_EXP NewGameEvent : public IEvent
	{
	protected:
		static const EventType m_EvtType;
	};

	class CRAZEENGINE_EXP MainMenuEvent : public IEvent
	{
	};

	class CRAZEENGINE_EXP ResetGameEvent : public IEvent
	{
	};

	class CRAZEENGINE_EXP GameOverEvent : public IEvent
	{
	};
}