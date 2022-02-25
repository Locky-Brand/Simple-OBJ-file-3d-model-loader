#include "OBJLoader.h"

namespace OBJFile {
	OBJLoader::OBJLoader() {

	}
	OBJLoader::~OBJLoader() {

	}

	loadInfo OBJLoader::load(const char* iPath, float* iRawArray) {
		std::ifstream file(iPath, std::ios_base::binary);
		if (file.is_open()) {
			std::vector<fileVec3> verts;
			std::vector<fileVec3> norms;
			std::vector<fileVec2> tex;
			std::vector<fileInd> inds;
			std::vector<fileInd> assembledInds;

			bool hasTex = false;
			bool hasNorm = false;

			// extraction of data
			std::string currentLine;
			while (!file.eof()) {
				std::getline(file, currentLine);
				switch (currentLine[0]) {
				case 'v':
				{
					switch (currentLine[1]) {
					case 'n':
					{
						fileVec3 vec;
						sscanf_s(currentLine.c_str(), "vn %f %f %f\n", &vec.x, &vec.y, &vec.z);
						norms.push_back(vec);
					}
					break;
					case 't':
					{
						fileVec2 vec;
						sscanf_s(currentLine.c_str(), "vt %f %f\n", &vec.x, &vec.y);
						tex.push_back(vec);
					}
					break;
					default:
					{
						fileVec3 vec;
						sscanf_s(currentLine.c_str(), "v %f %f %f\n", &vec.x, &vec.y, &vec.z);
						verts.push_back(vec);
					}
					break;
					}
				}
				break;
				case 'f':
				{
					fileInd ind = { 0, 0, 0 };
					unsigned int slashCount = 0;
					currentLine.append(1, '\n');
					std::string indStr = "";
					for (auto i = 2; i < currentLine.size(); ++i) {
						if (currentLine[i] == '/') {
							switch (slashCount)
							{
							case 0:
								ind.pos = atoi(indStr.c_str());
								indStr.clear();
								if (currentLine[i + 1] != '/') {
									hasTex = true;
								}
								break;
							case 1:
								ind.tex = atoi(indStr.c_str());
								indStr.clear();
								hasNorm = true;
								break;
							default:
								break;
							}
							++slashCount;
						}
						else if (currentLine[i] == ' ') {
							ind.norm = atoi(indStr.c_str());
							indStr.clear();
							inds.push_back(ind);
							slashCount = 0;
						}
						else if (currentLine[i] == '\n') {
							ind.norm = atoi(indStr.c_str());
							indStr.clear();
							inds.push_back(ind);
							slashCount = 0;
							fileInd pinInd = inds[0];
							for (int n = 2; n < assembledInds.size(); ++n) {
								assembledInds.push_back(pinInd);
								assembledInds.push_back(inds[n - 1]);
								assembledInds.push_back(inds[n]);
							}
							inds.clear();
						}
						else if (((currentLine[i] >= '0') && (currentLine[i] <= '9')) || (currentLine[i] == '-')) {
							indStr.append(1, currentLine[i]);
						}
					}
				}
				break;
				}
			}
			file.close();

			// triangulation and assembly of data
			if (hasTex && hasNorm) {
				std::vector <fileVertex_all> assembledVerts;

				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos].x, verts[i.pos].y, verts[i.pos].z, tex[i.tex].x, tex[i.tex].y, norms[i.norm].x, norms[i.norm].y, norms[i.norm].z });
				}

				iRawArray = new float[assembledVerts.size() * 8];
				for (auto i = 7; i < assembledVerts.size(); i += 8) {
					iRawArray[i - 7] = assembledVerts[i - 7].x;
					iRawArray[i - 6] = assembledVerts[i - 7].y;
					iRawArray[i - 5] = assembledVerts[i - 7].z;

					iRawArray[i - 4] = assembledVerts[i - 7].tx;
					iRawArray[i - 3] = assembledVerts[i - 7].ty;

					iRawArray[i - 2] = assembledVerts[i - 7].nx;
					iRawArray[i - 1] = assembledVerts[i - 7].ny;
					iRawArray[i] = assembledVerts[i - 7].nz;
				}
				return { loadInfo::dataLayout::POS_TEX_NORM, assembledVerts.size() * 8 };
			}
			else if (hasTex && !hasNorm) {
				std::vector <fileVertex_tex> assembledVerts;

				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos].x, verts[i.pos].y, verts[i.pos].z, tex[i.tex].x, tex[i.tex].y });
				}

				iRawArray = new float[assembledVerts.size() * 5];
				for (auto i = 4; i < assembledVerts.size(); i += 5) {
					iRawArray[i - 4] = assembledVerts[i - 4].x;
					iRawArray[i - 3] = assembledVerts[i - 4].y;
					iRawArray[i - 2] = assembledVerts[i - 4].z;

					iRawArray[i - 1] = assembledVerts[i - 4].tx;
					iRawArray[i] = assembledVerts[i - 4].ty;
				}
				return { loadInfo::dataLayout::POS_TEX, assembledVerts.size() * 5 };
			}
			else if (!hasTex && hasNorm) {
				std::vector <fileVertex_norm> assembledVerts;

				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos].x, verts[i.pos].y, verts[i.pos].z, norms[i.norm].x, norms[i.norm].y, norms[i.norm].z });
				}

				iRawArray = new float[assembledVerts.size() * 6];
				for (auto i = 5; i < assembledVerts.size(); i += 6) {
					iRawArray[i - 5] = assembledVerts[i - 5].x;
					iRawArray[i - 4] = assembledVerts[i - 5].y;
					iRawArray[i - 3] = assembledVerts[i - 5].z;

					iRawArray[i - 2] = assembledVerts[i - 5].nx;
					iRawArray[i - 1] = assembledVerts[i - 5].ny;
					iRawArray[i] = assembledVerts[i - 5].nz;
				}
				return { loadInfo::dataLayout::POS_NORM, assembledVerts.size() * 6 };
			}
			else {
				std::vector <fileVertex_pos> assembledVerts;

				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos].x, verts[i.pos].y, verts[i.pos].z });
				}

				iRawArray = new float[assembledVerts.size() * 3];
				for (auto i = 2; i < assembledVerts.size(); i += 3) {
					iRawArray[i - 2] = assembledVerts[i - 2].x;
					iRawArray[i - 1] = assembledVerts[i - 2].y;
					iRawArray[i] = assembledVerts[i - 2].z;
				}
				return { loadInfo::dataLayout::POS, assembledVerts.size() * 3 };
			}
		}
		else {
			std::cout << "error: file path not found" << '\n';
			return { loadInfo::dataLayout::ERROR, 0 };
		}
	}
	loadInfo OBJLoader::load(const char* iPath, std::vector<float>& iVector) {
		std::ifstream file(iPath, std::ios_base::binary);
		if (file.is_open()) {
			std::vector<fileVec3> verts;
			std::vector<fileVec3> norms;
			std::vector<fileVec2> tex;
			std::vector<fileInd> inds;
			std::vector<fileInd> assembledInds;

			bool hasTex = false;
			bool hasNorm = false;

			// extraction of data
			std::string currentLine;
			while (!file.eof()) {
				std::getline(file, currentLine);
				switch (currentLine[0]) {
				case 'v':
				{
					switch (currentLine[1]) {
					case 'n':
					{
						fileVec3 vec;
						sscanf_s(currentLine.c_str(), "vn %f %f %f\n", &vec.x, &vec.y, &vec.z);
						norms.push_back(vec);
					}
					break;
					case 't':
					{
						fileVec2 vec;
						sscanf_s(currentLine.c_str(), "vt %f %f\n", &vec.x, &vec.y);
						tex.push_back(vec);
					}
					break;
					default:
					{
						fileVec3 vec;
						sscanf_s(currentLine.c_str(), "v %f %f %f\n", &vec.x, &vec.y, &vec.z);
						verts.push_back(vec);
					}
					break;
					}
				}
				break;
				case 'f':
				{
					fileInd ind = { 0, 0, 0 };
					unsigned int slashCount = 0;
					currentLine.append(1, '\n');
					std::string indStr = "";
					for (auto i = 2; i < currentLine.size(); ++i) {
						if (currentLine[i] == '/') {
							switch (slashCount)
							{
							case 0:
								ind.pos = atoi(indStr.c_str());
								indStr.clear();
								if (currentLine[i + 1] != '/') {
									hasTex = true;
								}
								break;
							case 1:
								ind.tex = atoi(indStr.c_str());
								indStr.clear();
								hasNorm = true;
								break;
							default:
								break;
							}
							++slashCount;
						}
						else if (currentLine[i] == ' ') {
							ind.norm = atoi(indStr.c_str());
							indStr.clear();
							inds.push_back(ind);
							slashCount = 0;
						}
						else if (currentLine[i] == '\n') {
							ind.norm = atoi(indStr.c_str());
							indStr.clear();
							inds.push_back(ind);
							slashCount = 0;

							// triangulate all file inds in struct form
							fileInd pinInd = inds[0];
							for (int n = 2; n < inds.size(); ++n) {
								assembledInds.push_back(pinInd);
								assembledInds.push_back(inds[n - 1]);
								assembledInds.push_back(inds[n]);
							}
							inds.clear();
						}
						else if (((currentLine[i] >= '0') && (currentLine[i] <= '9')) || (currentLine[i] == '-')) {
							indStr.append(1, currentLine[i]);
						}
					}
				}
				break;
				}
			}
			file.close();

			// triangulation and assembly of data
			if (hasTex && hasNorm) {
				std::vector <fileVertex_all> assembledVerts;

				// get all verticies assembled
				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos - 1].x, verts[i.pos - 1].y, verts[i.pos - 1].z, tex[i.tex - 1].x, tex[i.tex - 1].y, norms[i.norm - 1].x, norms[i.norm - 1].y, norms[i.norm - 1].z });
				}

				// push all floats into the input buffer
				for (auto i = 0; i < assembledVerts.size(); ++i) {
					iVector.push_back(assembledVerts[i].x);
					iVector.push_back(assembledVerts[i].y);
					iVector.push_back(assembledVerts[i].z);

					iVector.push_back(assembledVerts[i].tx);
					iVector.push_back(assembledVerts[i].ty);

					iVector.push_back(assembledVerts[i].nx);
					iVector.push_back(assembledVerts[i].ny);
					iVector.push_back(assembledVerts[i].nz);
				}

				return { loadInfo::dataLayout::POS_TEX_NORM, iVector.size() };
			}
			else if (hasTex && !hasNorm) {
				std::vector <fileVertex_tex> assembledVerts;

				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos - 1].x, verts[i.pos - 1].y, verts[i.pos - 1].z, tex[i.tex - 1].x, tex[i.tex - 1].y });
				}

				for (auto i = 0; i < assembledVerts.size(); ++i) {
					iVector.push_back(assembledVerts[i].x);
					iVector.push_back(assembledVerts[i].y);
					iVector.push_back(assembledVerts[i].z);

					iVector.push_back(assembledVerts[i].tx);
					iVector.push_back(assembledVerts[i].ty);
				}

				return { loadInfo::dataLayout::POS_TEX, iVector.size() };
			}
			else if (!hasTex && hasNorm) {
				std::vector <fileVertex_norm> assembledVerts;

				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos - 1].x, verts[i.pos - 1].y, verts[i.pos - 1].z, norms[i.norm - 1].x, norms[i.norm - 1].y, norms[i.norm - 1].z });
				}

				for (auto i = 0; i < assembledVerts.size(); ++i) {
					iVector.push_back(assembledVerts[i].x);
					iVector.push_back(assembledVerts[i].y);
					iVector.push_back(assembledVerts[i].z);

					iVector.push_back(assembledVerts[i].nx);
					iVector.push_back(assembledVerts[i].ny);
					iVector.push_back(assembledVerts[i].nz);
				}

				return { loadInfo::dataLayout::POS_NORM, iVector.size() };
			}
			else {
				std::vector <fileVertex_pos> assembledVerts;

				for (auto& i : assembledInds) {
					assembledVerts.push_back({ verts[i.pos - 1].x, verts[i.pos - 1].y, verts[i.pos - 1].z });
				}

				for (auto i = 0; i < assembledVerts.size(); ++i) {
					iVector.push_back(assembledVerts[i].x);
					iVector.push_back(assembledVerts[i].y);
					iVector.push_back(assembledVerts[i].z);
				}

				return { loadInfo::dataLayout::POS, iVector.size() };
			}
		}
		else {
			std::cout << "error: file path not found" << '\n';
			return { loadInfo::dataLayout::ERROR, 0 };
		}
	}
}