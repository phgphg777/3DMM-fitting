#pragma once
#include <vector>
#include <array>

struct simple_mesh
{
	std::vector< std::array<float,3> > V;
	std::vector< std::array<int,3> > I;
	std::vector< std::array<float,3> > N; 
	std::vector< std::array<int,3> > NI;
};


std::vector<std::array<float,3>> get_average_noraml(
	const std::vector<std::array<float,3>>& vertices,
	const std::vector<std::array<int,3>>& topology)
{
	auto& V = vertices;
	auto& I = topology;
	int numv = (int) V.size();
	int numf = (int) I.size();

	std::vector<std::array<float,3>> face_normal(numf);
	std::array<float,3> v, w, n;
	for (int i = 0; i < numf; i++) 
	{
		v[0] = V[I[i][1]] [0] - V[I[i][0]] [0];
		v[1] = V[I[i][1]] [1] - V[I[i][0]] [1];
		v[2] = V[I[i][1]] [2] - V[I[i][0]] [2];

		w[0] = V[I[i][2]] [0] - V[I[i][0]] [0];
		w[1] = V[I[i][2]] [1] - V[I[i][0]] [1];
		w[2] = V[I[i][2]] [2] - V[I[i][0]] [2];

		n[0] = v[1]*w[2] - v[2]*w[1];
		n[1] = v[2]*w[0] - v[0]*w[2];
		n[2] = v[0]*w[1] - v[1]*w[0];

		float norm = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		face_normal[i] = { n[0]/norm, n[1]/norm, n[2]/norm };
	}

	
	std::vector<std::array<float,3>> vertex_normal(numv, {0, 0, 0});
	for (int i = 0; i < numf; ++i) 
	{
		for (int j = 0; j < 3; ++j) 
		{
			vertex_normal[I[i][j]] [0] += face_normal[i][0];
			vertex_normal[I[i][j]] [1] += face_normal[i][1];
			vertex_normal[I[i][j]] [2] += face_normal[i][2];
		}
	}
	for (int i = 0; i < numv; ++i)
	{
		auto& n = vertex_normal[i];
		float norm = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		vertex_normal[i] = { n[0]/norm, n[1]/norm, n[2]/norm };
	}

	return vertex_normal;
}