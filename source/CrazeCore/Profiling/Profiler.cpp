#include "Profiler.h"
#include <fstream>
#include <string>

#include <iostream>

using namespace Craze;

Profiler Craze::gProfiler;

Profiler::Profiler()
{
	m_IsInFrame = false;
	m_pCurrentNode = 0;
}

void Profiler::PushNode(const char* nodeId)
{
	if (m_IsInFrame)
	{
		ProfilerNode* pNode = new(m_NodePool.malloc()) ProfilerNode();
		pNode->m_pNodeId = nodeId;
		pNode->m_StartTime = m_Clock.GetClock();

		m_pCurrentNode->AppendChild(pNode);

		m_pCurrentNode = pNode;
	}
}
void Profiler::PopNode()
{
	if (m_IsInFrame)
	{
		if (m_pCurrentNode != m_pCurrentRoot)
		{
			m_pCurrentNode->m_StopTime = m_Clock.GetClock();
			m_pCurrentNode = m_pCurrentNode->m_pParent;
		}
	}
}

void Profiler::BeginFrame(const char* frameName)
{
	if (m_IsInFrame)
		return;

	ProfilerNode* pNode = new(m_NodePool.malloc()) ProfilerNode();
	pNode->m_pParent = 0;
	pNode->m_pNodeId = frameName;
	pNode->m_StartTime = m_Clock.GetClock();
	
	m_RootNodes.push_back(pNode);
	m_pCurrentRoot = m_pCurrentNode = pNode;

	m_IsInFrame = true;
}
void Profiler::StopFrame()
{
	if (!m_IsInFrame)
		return;

	m_IsInFrame = false;
	m_pCurrentNode = 0;
	m_pCurrentRoot->m_StopTime = m_Clock.GetClock();
	m_pCurrentRoot = 0;
}

bool sortByName(NodeData n1, NodeData n2)
{
	return strcmp(n1.m_pNodeId, n2.m_pNodeId) < 0;
}

bool sortByDelta(NodeData n1, NodeData n2)
{
	return n1.m_Delta > n2.m_Delta;
}

bool sortByInNode(NodeData n1, NodeData n2)
{
	return n1.m_InNode > n2.m_InNode;
}

void PrintStatistics(std::vector<NodeData>& nodes, std::ofstream& file, int id)
{
	if (nodes.size() == 0)
	{
		return;
	}

	std::vector<NodeData> sortedByName = nodes;

	std::sort(sortedByName.begin(), sortedByName.end(), sortByName);
	

	std::vector<NodeData> concNames;
	concNames.push_back(sortedByName[0]);

	//Concatenate all the same strings
	for (unsigned int i = 1; i < sortedByName.size(); ++i)
	{
		if (strcmp(concNames.back().m_pNodeId, sortedByName[i].m_pNodeId) == 0)
		{
			concNames.back().m_Delta += sortedByName[i].m_Delta;
			concNames.back().m_InNode += sortedByName[i].m_InNode;
		} else
		{
			concNames.push_back(sortedByName[i]);
		}
	}


	std::vector<NodeData> sortedByDelta = nodes;
	std::vector<NodeData> sortedByTimeInNode = nodes;
	std::vector<NodeData> sortedByTotalDelta = concNames;
	std::vector<NodeData> sortedByTotalTimeInNode = concNames;
	
	std::sort(sortedByDelta.begin(), sortedByDelta.end(), sortByDelta);
	std::sort(sortedByTimeInNode.begin(), sortedByTimeInNode.end(), sortByInNode);
	std::sort(sortedByTotalDelta.begin(), sortedByTotalDelta.end(), sortByDelta);
	std::sort(sortedByTotalTimeInNode.begin(), sortedByTotalTimeInNode.end(), sortByInNode);
		
	/* TIME IN EACH NODE INCLUDING SUBNODES */
	
	file << "<p><font size='+1'>Time spent in each node, each call(including sub nodes):</font><br />\n";
	file << "<div style='height: 500px; width: 1024px; overflow: scroll;' onclick=\"toggleSection('" << id << ".50')\" id='" << id << ".50'>";
	file << "<table border='1' cellpadding='1' cellspacing='0' bordercolor='#CCCCCC'>\n";
	
	unsigned long total = 0;
	
	for (auto i = concNames.begin(); i != concNames.end(); ++i)
	{
		total += (*i).m_InNode;
	}

	for (auto i = sortedByDelta.begin(); i != sortedByDelta.end(); ++i)
	{

		file << "<tr><td>" << (*i).m_Delta << "</td><td>" << (*i).m_pNodeId << "</td><td>" << (int)(((float)(*i).m_Delta / (float)total) * 100) << "%</td></tr>\n";
	}
	
	file << "</table></div>\n";
	
	/* TIME IN EACH NODE NOT INCLUDING SUBNODES */
	file << "<p><font size='+1'>Time spent in each node, each call (excluding sub nodes):</font><br />\n";
	file << "<div style='height: 500px; width: 1024px; overflow: scroll;' onclick=\"toggleSection('" << id << ".51')\" id='" << id << ".51'>";
	file << "<table border='1' cellpadding='1' cellspacing='0' bordercolor='#CCCCCC'>\n";
	

	for (auto i = sortedByTimeInNode.begin(); i != sortedByTimeInNode.end(); ++i)
	{
		file << "<tr><td>" << (*i).m_InNode << "</td><td>" << (*i).m_pNodeId << "</td><td>" << (int)(((float)(*i).m_InNode / (float)total) * 100) << "%</td></tr>\n";
	}

	file << "</table></div>\n";

	/* Time spent in nodes, over all frame (including sub nodes): */
	file << "<p><font size='+1'>Time spent in nodes, over all frame (including sub nodes):</font><br />\n";
	file << "<div style='height: 500px; width: 1024px; overflow: scroll;' onclick=\"toggleSection('" << id << ".52')\" id='" << id << ".52'>";
	file << "<table border='1' cellpadding='1' cellspacing='0' bordercolor='#CCCCCC'>\n";
	
	for (auto i = sortedByTotalDelta.begin(); i != sortedByTotalDelta.end(); ++i)
	{
		file << "<tr><td>" << (*i).m_Delta << "</td><td>" << (*i).m_pNodeId << "</td><td>" << (int)(((float)(*i).m_Delta / (float)total) * 100) << "%</td></tr>\n";
	}

	file << "</table></div>\n";
	/* Time spent in nodes, over all frame (excluding sub nodes): */
	file << "<p><font size='+1'>Time spent in nodes, over all frame (excluding sub nodes):</font><br />\n";
	file << "<div style='height: 500px; width: 1024px; overflow: scroll;' onclick=\"toggleSection('" << id << ".53')\" id='" << id << ".53'>";
	file << "<table border='1' cellpadding='1' cellspacing='0' bordercolor='#CCCCCC'>\n";
	
	for (auto i = sortedByTotalTimeInNode.begin(); i != sortedByTotalTimeInNode.end(); ++i)
	{
		file << "<tr><td>" << (*i).m_InNode << "</td><td>" << (*i).m_pNodeId << "</td><td>" << (int)(((float)(*i).m_InNode / (float)total) * 100) << "%</td></tr>\n";
	}
	file << "</table></div>\n";
}

void Profiler::PrintStatisticsToFile(std::string fileName)
{
	if (m_IsInFrame || m_RootNodes.size() == 0)
		return;

	std::ofstream outFile;
	outFile.open(fileName.c_str(), std::ios_base::out | std::ios_base::trunc);
	std::vector<NodeData> allNodes;

	if (!outFile.is_open())
	{
		return;
	}
	

#ifdef CRAZE_PROFILING_DISABLED
	outFile << "<html><head><title>Craze profiler</title></head><body><h1>Craze profiling disabled</h1></body></html>\n";
#else

	std::cout << "Generating Craze profiler log\n";
	outFile << "<html><head><title>Craze profiler</title>";
	outFile << "<script language='javascript'><!-- \n";
	outFile << "function toggleSection(id) { \nif (document.getElementById(id).style.height == 'auto') {\n";
	outFile << "document.getElementById(id).style.height = 500;\n } else {\ndocument.getElementById(id).style.height = 'auto';\n}\n}\n//-->\n</script>\n";
	outFile << "</head><body><h1>Craze profiler run log</h1><p><strong><font size='+2'>Index</font></strong></p>\n";
	
	for (int i = 0; i < (int)m_RootNodes.size(); ++i)
	{
		outFile << "<a href='#" << i << "'>" << m_RootNodes[i]->m_pNodeId << "</a><br />\n";
		outFile << "&nbsp;&nbsp;<a href='#" << i << ".1'>Frame calls</a><br />\n";
		outFile << "&nbsp;&nbsp;<a href='#" << i << ".2'>Frame stats</a><br />\n";
		outFile << "&nbsp;&nbsp;<a href='#" << i << ".3'>Frame graph</a><br />\n";
	}
	
	outFile << "<hr />\n";
	
	for (int i = 0; i < (int)m_RootNodes.size(); ++i)
	{
		std::cout << "Printing frame " << i + 1 << " of " << m_RootNodes.size() << ".\n";
		outFile << "<font size='+2'><a name='" << i << ".1'>Frame: <b>" << m_RootNodes[i]->m_pNodeId << " </b></a></font>\n";
		outFile << "<p>Total time in frame: <b>" << (m_RootNodes[i]->m_StopTime - m_RootNodes[i]->m_StartTime) << "</b></p>\n";

		outFile << "<p><font size='+1'><a name='" << i << "'>Function call list:</a></font></p>\n<textarea rows='20' cols='120'>\n";

		for (ProfilerNode* j = m_RootNodes[i]->m_pFirstChild; j; j = j->m_pNextSibling)
		{
			PrintNode(&(*j), outFile, "  ", allNodes);
		}

		outFile << "</textarea>\n";

		outFile << "<p><font size='+1'><b><a name='" << i <<".2'>Frame stats</a></b></font></p>\n";
		PrintStatistics(allNodes, outFile, i);

		allNodes.clear();
		
		outFile << "<hr />\n";

	}

	outFile << "</body></html>\n";

#endif

	outFile.close();


}

void Profiler::PrintNode(ProfilerNode *pNode, std::ofstream &file, std::string tabStr, std::vector<NodeData>& allNodes)
{
	if (m_IsInFrame)
		return;

	NodeData data;
	data.m_pNodeId = pNode->m_pNodeId;
	data.m_Delta = pNode->m_StopTime - pNode->m_StartTime;
	data.m_InNode = data.m_Delta;

	for (auto i = pNode->m_pFirstChild; i; i = i->m_pNextSibling)
	{
		data.m_InNode -= (i->m_StopTime - i->m_StartTime);
	}

	allNodes.push_back(data);

	file << tabStr << pNode->m_pNodeId << "\n";
	file << tabStr << "Total: " << data.m_Delta << "\n";
	file << tabStr << "Time without subnodes: " << data.m_InNode << "\n";

	file << "\n\n";

	for (auto i = pNode->m_pFirstChild; i; i = i->m_pNextSibling)
	{
		PrintNode(i, file, tabStr + "  ", allNodes);
	}
}

CrazeProfilerHelper::CrazeProfilerHelper(const char* pStr)
{ 
	PROFILER_BEGIN(pStr); 
}
CrazeProfilerHelper::~CrazeProfilerHelper()
{ 
	PROFILER_END(); 
}