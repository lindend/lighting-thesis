#pragma once
#ifdef _WIN32
#include "Win32Clock.h"
#endif

#include <vector>

#include "ProfilerNode.h"
#include "../Memory/MemoryManager.h"

#ifndef CRAZE_PROFILING_DISABLE
#define PROFILER_BEGIN(functionName) Craze::gProfiler.PushNode(functionName)
#define PROFILER_END() Craze::gProfiler.PopNode()
#define PROFILEF() Craze::CrazeProfilerHelper craze_local_profile_helper(__FUNCTION__);
#define PROFILE(functname) Craze::CrazeProfilerHelper craze_local_profile_helper(functname);
#else
#define PROFILER_BEGIN(functionName)
#define PROFILER_END()
#define PROFILE(functname);
#define PROFILEF()
#endif


namespace Craze
{
	class ProfilerNode
	{
	public:
		ProfilerNode() : m_pFirstChild(0), m_pNextSibling(0), m_pParent(0), m_pLastChild(0) {}

		ProfilerNode* m_pParent;
		ProfilerNode* m_pNextSibling;
		ProfilerNode* m_pFirstChild;
		ProfilerNode* m_pLastChild;

		void AppendChild(ProfilerNode* pChild)
		{
			if (m_pLastChild)
			{
				m_pLastChild->m_pNextSibling = pChild;
			} else
			{
				m_pFirstChild = pChild;
			}

			pChild->m_pParent = this;
			m_pLastChild = pChild;
		}

		unsigned long m_StartTime;
		unsigned long m_StopTime;

		const char* m_pNodeId;
	};

	struct NodeData
	{
		unsigned long m_Delta;
		unsigned long m_InNode;
		
		const char* m_pNodeId;
	};

	typedef std::vector<ProfilerNode*> NodeList;

	extern class Profiler gProfiler;

	/**
	Class for profiling the application. Simply use the PROFILE(functionName) macro at the top of the function and it will be added
	to the profilers report.
	*/
	class Profiler
	{
		friend bool InitProfiler();

	public:
		Profiler();
		~Profiler() {}

		/*
		PushNode
		Adds a new subnode to the frame.
		*/
		void PushNode(const char* nodeId);
		/*
		PopNode
		Steps up one step in the node hierarchy
		*/
		void PopNode();

		/*
		BeginFrame
		Starts a new profiling frame.
		*/
		void BeginFrame(const char* frameName);
		/*
		StopFrame
		Stops the current frame.
		*/
		void StopFrame();

		/*
		PrintStatisticsToFile
		Saves all stored frames into a file.
		*/
		void PrintStatisticsToFile(std::string fileName);

	private:
		boost::object_pool<ProfilerNode> m_NodePool;

		NodeList m_RootNodes;
		ProfilerNode* m_pCurrentNode;
		ProfilerNode* m_pCurrentRoot;
		Win32Clock m_Clock;

		bool m_IsInFrame;

		void PrintNode(ProfilerNode* pNode, std::ofstream& file, std::string tabStr, std::vector<NodeData>& allNodes);

	};

	class CrazeProfilerHelper
	{
	public:
		CrazeProfilerHelper(const char* pStr);
		~CrazeProfilerHelper();
	};
}