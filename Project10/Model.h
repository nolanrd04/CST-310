#pragma once
// Std. Includes
#include <string> // Include string
#include <fstream> // include fstream
#include <sstream> // Include sstream
#include <iostream> // Include iostream
#include <map> // Include map
#include <vector> // Include vector
using namespace std; // Use namespace std
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp> // Include glm
#include <glm/gtc/matrix_transform.hpp> // Include matrix transform
#include <SOIL/SOIL.h> // Include SOIL
#include <assimp/Importer.hpp> // Include assimp importer
#include <assimp/scene.h> // Include assimp scene
#include <assimp/postprocess.h> // Include assimp postprocess

#include "Mesh.h" // Include Mesh.h

GLint TextureFromFile(const char* path, string directory); // Texture from file

class Model  // Provided in class
{
public:
	/*  Functions   */
	// Constructor, expects a filepath to a 3D model.
	Model(GLchar* path) // Model constructor using path
	{
		this->loadModel(path); // Load model with callback and path
	}
	
	// Draws the model, and thus all its meshes
	void Draw(Shader shader)
	{
		for(GLuint i = 0; i < this->meshes.size(); i++) // Iterate over mesh
			this->meshes[i].Draw(shader); // Draw
	}
	
private:
	/*  Model Data  */
	vector<Mesh> meshes; // Vector of meshes
	string directory; // String for directory
	
	/*  Functions   */
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string path)
	{
		// Read file via ASSIMP
		Assimp::Importer importer; // Initialize importer
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs); // Read model
		// Check for errors
		if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl; // Write error message
			return;
		}
		// Retrieve the directory path of the filepath
		this->directory = path.substr(0, path.find_last_of('/')); // Get directory
		
		// Process ASSIMP's root node recursively
		this->processNode(scene->mRootNode, scene); // Process nodes using callback
	}
	
	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process each mesh located at the current node
		for(GLuint i = 0; i < node->mNumMeshes; i++) // Iterate over mNumMeshes
		{
			// The node object only contains indices to index the actual objects in the scene. 
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; // Assign mesh
			this->meshes.push_back(this->processMesh(mesh, scene)); // Push mesh back to meshes using processMesh method
		}
		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for(GLuint i = 0; i < node->mNumChildren; i++) // Iterate over children
		{
			this->processNode(node->mChildren[i], scene); // Process child nodes
		}
		
	}
	
	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		// Data to fill
		vector<Vertex> vertices; // Vector for vertices
		vector<GLuint> indices; // Vector for indices
		vector<Texture> textures; // Vector for textures
		
		// Walk through each of the mesh's vertices
		for(GLuint i = 0; i < mesh->mNumVertices; i++) // Iterate over vertices
		{
			Vertex vertex; // Initialize vertex
			glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
			// Positions
			vector.x = mesh->mVertices[i].x; // Set x based on mesh
			vector.y = mesh->mVertices[i].y; // Set y based on mesh
			vector.z = mesh->mVertices[i].z; // Set z based on mesh
			vertex.Position = vector; // Set position = vector
			// Normals
			vector.x = mesh->mNormals[i].x; // Set normal x based on mesh
			vector.y = mesh->mNormals[i].y; // Set normal y based on mesh
			vector.z = mesh->mNormals[i].z; // Set normal z based on mesh
			vertex.Normal = vector; // Set vertex.Normal based on vector
			// Texture Coordinates
			if(mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
			{
				glm::vec2 vec; // Initialize vec2
				// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x; // Set x for texcoords
				vec.y = mesh->mTextureCoords[0][i].y; // Set y for texcoords
				vertex.TexCoords = vec; // Set texcoords
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f); // Set texcoords as 0.0
			vertices.push_back(vertex); // Push vertex back to vertices
		}
		// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for(GLuint i = 0; i < mesh->mNumFaces; i++) // Iterate over faces
		{
			aiFace face = mesh->mFaces[i]; // Set face
			// Retrieve all indices of the face and store them in the indices vector
			for(GLuint j = 0; j < face.mNumIndices; j++) // Iterate over face indices
				indices.push_back(face.mIndices[j]); // Push back face indices
		}
		
		// Calculate tangents and bitangents
		this->calculateTangentsBitangents(vertices, indices); // Calculate tangents and bitangents
		
		// Process materials
		if(mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex]; // Set material
			// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			// Same applies to other texture as the following list summarizes:
			// Diffuse: texture_diffuseN
			// Specular: texture_specularN
			// Normal: texture_normalN
			
			// 1. Diffuse maps
			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse"); // Set diffuseMaps
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end()); // Insert textures
			// 2. Specular maps
			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular"); // Set specularMaps
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end()); // Insert textures
		}
		
		// Return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures); // Return Mesh object from vertices, indices, textures defined above
	}
	
	// Calculate tangents and bitangents for all vertices based on triangle data
	void calculateTangentsBitangents(vector<Vertex>& vertices, vector<GLuint>& indices)
	{
		// Initialize all tangents and bitangents to zero
		for(GLuint i = 0; i < vertices.size(); i++) // Iterate over vertices
		{
			vertices[i].Tangent = glm::vec3(0.0f); // Initialize tangent to 0.0f
			vertices[i].Bitangent = glm::vec3(0.0f); // Initialize bitangent to 0.0f
		}
		
		// Calculate tangent and bitangent for each triangle
		for(GLuint i = 0; i < indices.size(); i += 3) // Iterate over triangles (3 indices per triangle)
		{
			GLuint i1 = indices[i]; // Get first index
			GLuint i2 = indices[i + 1]; // Get second index
			GLuint i3 = indices[i + 2]; // Get third index
			
			glm::vec3 v1 = vertices[i1].Position; // Get first vertex position
			glm::vec3 v2 = vertices[i2].Position; // Get second vertex position
			glm::vec3 v3 = vertices[i3].Position; // Get third vertex position
			
			glm::vec2 uv1 = vertices[i1].TexCoords; // Get first vertex texture coordinate
			glm::vec2 uv2 = vertices[i2].TexCoords; // Get second vertex texture coordinate
			glm::vec2 uv3 = vertices[i3].TexCoords; // Get third vertex texture coordinate
			
			glm::vec3 e1 = v2 - v1; // Calculate edge 1
			glm::vec3 e2 = v3 - v1; // Calculate edge 2
			
			glm::vec2 duv1 = uv2 - uv1; // Calculate texture coordinate delta 1
			glm::vec2 duv2 = uv3 - uv1; // Calculate texture coordinate delta 2
			
			float f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y); // Calculate denominator
			
			// Calculate tangent
			glm::vec3 tangent;
			tangent.x = f * (duv2.y * e1.x - duv1.y * e2.x); // Calculate tangent x
			tangent.y = f * (duv2.y * e1.y - duv1.y * e2.y); // Calculate tangent y
			tangent.z = f * (duv2.y * e1.z - duv1.y * e2.z); // Calculate tangent z
			
			// Calculate bitangent
			glm::vec3 bitangent;
			bitangent.x = f * (-duv2.x * e1.x + duv1.x * e2.x); // Calculate bitangent x
			bitangent.y = f * (-duv2.x * e1.y + duv1.x * e2.y); // Calculate bitangent y
			bitangent.z = f * (-duv2.x * e1.z + duv1.x * e2.z); // Calculate bitangent z
			
			// Accumulate tangent and bitangent to each vertex of the triangle
			vertices[i1].Tangent += tangent; // Add tangent to first vertex
			vertices[i2].Tangent += tangent; // Add tangent to second vertex
			vertices[i3].Tangent += tangent; // Add tangent to third vertex
			
			vertices[i1].Bitangent += bitangent; // Add bitangent to first vertex
			vertices[i2].Bitangent += bitangent; // Add bitangent to second vertex
			vertices[i3].Bitangent += bitangent; // Add bitangent to third vertex
		}
		
		// Normalize tangents and bitangents
		for(GLuint i = 0; i < vertices.size(); i++) // Iterate over vertices
		{
			vertices[i].Tangent = glm::normalize(vertices[i].Tangent); // Normalize tangent
			vertices[i].Bitangent = glm::normalize(vertices[i].Bitangent); // Normalize bitangent
		}
	}
	
	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures; // Initialize textures
		for(GLuint i = 0; i < mat->GetTextureCount(type); i++) // Iterate over textures
		{
			aiString str; // Initialize aiString
			mat->GetTexture(type, i, &str); // Get texture using type and string
			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			Texture texture; // Initialize texture
			texture.id = TextureFromFile(str.C_Str(), this->directory); // Assign id
			texture.type = typeName; // Assign type
			texture.path = str; // Assign path
			textures.push_back(texture); // Push back texture
		}
		return textures; // Return vector of textures
	}
};



GLint TextureFromFile(const char* path, string directory)
{
	//Generate texture ID and load texture data 
	string filename = string(path); // Get filename
	filename = directory + '/' + filename; // Get filename with directory
	GLuint textureID; // Initialize textureID
	glGenTextures(1, &textureID); // Gen textures with textureID
	int width,height; // Initialize width and height
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB); // Get SOIL image
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID); // Bind texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); // Bind image
	glGenerateMipmap(GL_TEXTURE_2D); // Generate mip maps
	
	// Parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // Set texture wrap s
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // Set texture wrap t
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); // Set min filter
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Set mag filter
	glBindTexture(GL_TEXTURE_2D, 0); // Bind texture
	SOIL_free_image_data(image); // Use image
	return textureID; // Return textureID
}
