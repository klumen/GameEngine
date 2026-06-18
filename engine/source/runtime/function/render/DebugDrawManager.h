#pragma once

namespace Lumen
{
	class DebugDrawManager
	{
	public:
		DebugDrawManager() {};
		~DebugDrawManager() {};

		void StartUp();
		void ShutDown();

		// TODO:
		/*void AddLine(const Point& fromPosition,
			const Point& toPosition,
			Color color,
			float lineWidth = 1.0f,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds an axis-aligned cross (3 lines converging at
		// a point) to the debug drawing queue.
		void AddCross(const Point& position,
			Color color,
			float size,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds a wireframe sphere to the debug drawing queue.
		void AddSphere(const Point& centerPosition,
			float radius,
			Color color,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds a circle to the debug drawing queue.
		void AddCircle(const Point& centerPosition,
			const Vector& planeNormal,
			float radius,
			Color color,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds a set of coordinate axes depicting the
		// position and orientation of the given
		// transformation to the debug drawing queue.
		void AddAxes(const Transform& xfm,
			Color color,
			float size,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds a wireframe triangle to the debug drawing
		// queue.
		void AddTriangle(const Point& vertex0,
			const Point& vertex1,
			const Point& vertex2,
			Color color,
			float lineWidth = 1.0f,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds an axis-aligned bounding box to the debug
		// queue.
		void AddAABB(const Point& minCoords,
			const Point& maxCoords,
			Color color,
			float lineWidth = 1.0f,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds an oriented bounding box to the debug queue.
		void AddOBB(const Mat44& centerTransform,
			const Vector& scaleXYZ,
			Color color,
			float lineWidth = 1.0f,
			float duration = 0.0f,
			bool depthEnabled = true);
		// Adds a text string to the debug drawing queue.
		void AddString(const Point& pos,
			const char* text,
			Color color,
			float duration = 0.0f,
			bool depthEnabled = true);*/

	private:
		DebugDrawManager(const DebugDrawManager&) = delete;
		DebugDrawManager& operator=(const DebugDrawManager&) = delete;

	};
}