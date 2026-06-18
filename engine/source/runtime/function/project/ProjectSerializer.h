#pragma once

#include "runtime/function/project/Project.h"

namespace Lumen
{
	class ProjectSerializer
	{
	public:
		ProjectSerializer(Shared<Project> project);

		bool Serialize();
		bool Deserialize();

	private:
		Shared<Project> m_Project;

	};
}