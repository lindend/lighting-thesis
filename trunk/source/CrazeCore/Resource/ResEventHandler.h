#pragma once

namespace Craze
{
	class Resource;

	class ResEventHandler
	{
	public:
		virtual ~ResEventHandler() {}

		//Returning false from any of the handlers below will cancel the file loading and flag the resource with an error

		//Called before the resource starts to load, from the thread that started the loading of the resource
		virtual bool preRead(Resource* res) = 0;
		//Called once the resource has been fully loaded, this is when you want to start loading subresources
		virtual bool readComplete(Resource* res) = 0;
		//Called when all of the subresources has been fully loaded
		virtual bool allComplete(Resource* res) = 0;

		//If there are errors reading the file, this function will be called. Return true from this to continue processing anyway
		virtual bool fileReadError(Resource* res) = 0;

		bool readCompleteMT() const { return m_readCompleteMT; }
		bool allCompleteMT() const { return m_allCompleteMT; }
	protected:
		bool m_readCompleteMT;
		bool m_allCompleteMT;
	};

	class DefaultResHandler : public ResEventHandler
	{
	public:
		DefaultResHandler() { m_readCompleteMT = true; m_allCompleteMT = true;}

		virtual bool preRead(Resource* res) { return true; }
		virtual bool readComplete(Resource* res) { return true; }
		virtual bool allComplete(Resource* res) { return true; }

		virtual bool fileReadError(Resource* res) { return true; }

	};
}
