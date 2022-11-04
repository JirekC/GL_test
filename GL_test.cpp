#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>
#include <stdint.h>
#include <unistd.h> // readlink()
#include <glad/glad.h> // OpenGL loader
#include <GLFW/glfw3.h> // OpenGL window & input
#include <glm/glm.hpp> // OpenGL math (C++ wrap)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "nlohmann/json.hpp" // JSON parser for project files

#include "f3d/scanner.hpp"
#include "f3d/grid.hpp"
#include "f3d/object_creator.hpp"
#include "f3d/material_map.hpp"
#include "f3d/shader.hpp"

using json = nlohmann::json;

#ifndef M_PI
    #define M_PI 3.14159265358979323846264338327950288
#endif /* M_PI */

constexpr float frame_time = 20.0e-3f; // 1 / desired_fps
constexpr float rotation_sensitivity = 0.005f; // TODO: let user to adjust
float lastX, lastY;
float cam_yaw = 0.0f;
float cam_pitch = 0.0f; //was 3.14f * 0.25f;
unsigned int frame = 0;
unsigned int num_frames;

// separate thread: waiting for commands via std input
std::string cmd_line;
std::atomic<int> cmd_flag { 0 };
void cmd_input_thread(void)
{
    std::string s;
    while(std::getline(std::cin, s, '\n'))
    {
        cmd_line = std::move(s);
        if (cmd_line == "quit") {
            break;
        }
        cmd_flag = 1; // mark new command available
        while(cmd_flag > 0)
        {
            // wait for main code to read command and reset flag
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    cmd_flag = -1; // mark end-of-work
}

template<typename T>
glm::vec<3, T> parse_vec3(json jvec3)
{
    glm::vec<3, T> ret_vec;
    json val;

    if(!jvec3.is_object())
    {
        throw std::runtime_error("3D vector not found");
    }
    if(!(val = jvec3["x"]).is_number())
    {
        throw std::runtime_error("invalid 3D vector format");
    }
    ret_vec.x = val;
    if(!(val = jvec3["y"]).is_number())
    {
        throw std::runtime_error("invalid 3D vector format");
    }
    ret_vec.y = val;
    if(!(val = jvec3["z"]).is_number())
    {
        throw std::runtime_error("invalid 3D vector format");
    }
    ret_vec.z = val;

    return ret_vec;
}

template<typename T>
glm::vec<2, T> parse_vec2(json jvec2)
{
    glm::vec<2, T> ret_vec;
    json val;

    if(!jvec2.is_object())
    {
        throw std::runtime_error("3D vector not found");
    }
    if(!(val = jvec2["x"]).is_number())
    {
        throw std::runtime_error("invalid 3D vector format");
    }
    ret_vec.x = val;
    if(!(val = jvec2["y"]).is_number())
    {
        throw std::runtime_error("invalid 3D vector format");
    }
    ret_vec.y = val;

    return ret_vec;
}

std::string getexepath()
{
    char result[ 1024 ];
    ssize_t count = readlink( "/proc/self/exe", result, sizeof(result) );
    std::string path( result, (count > 0) ? count : 0 );
    size_t found = path.find_last_of("/\\");
    path = path.substr(0,found);
    return path;
}

// processing inputs (from main loop)
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// mouse move - rotation callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static bool first_round = true;
    if (first_round)
    {
        lastX = xpos;
        lastY = ypos;
        first_round = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed: y ranges bottom to top
    lastX = xpos;
    lastY = ypos;
    
    cam_yaw += xoffset * rotation_sensitivity;
    cam_pitch += yoffset * rotation_sensitivity;
    if (cam_pitch >= 0.49f * M_PI) cam_pitch = 0.49f * M_PI;
    if (cam_pitch <= -0.49f * M_PI) cam_pitch = -0.49f * M_PI;
}

// mouse scroll - frame inc / dec
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    int inc = (int)yoffset * (1 + num_frames / 100);
    
    if ((int)frame + inc >= 0)
        frame += inc;
}

void printProgramInfo(GLuint nProgram)
{
    
    int nLen = 0;
    int nWrittn = 0;
    char* chLog = nullptr;
    glGetProgramiv(nProgram, GL_INFO_LOG_LENGTH, &nLen);
    if (nLen > 0)
    {
    
        chLog = (char*)malloc(nLen);
        if (!chLog)
        {
    
            std::cout << "Allocate memory failed! Invoke malloc()." << std::endl;
            return;
        }
        glGetProgramInfoLog(nProgram, nLen, &nWrittn, chLog);
        std::cout << "Program infomation log:" << chLog << std::endl;
        free(chLog);
    }
}

// translates material number to color from palette
glm::vec4 ColorFromMaterial(uint8_t material)
{
    glm::vec4 color;
    const glm::vec4 palette[9] = {  {0.1f, 0.1f, 0.1f, 1.0f}, // mat #0
                                    {0.0f, 0.0f, 1.0f, 1.0f},
                                    {0.0f, 1.0f, 0.0f, 1.0f},
                                    {0.0f, 1.0f, 1.0f, 1.0f},
                                    {1.0f, 0.0f, 0.0f, 1.0f},
                                    {1.0f, 0.0f, 1.0f, 1.0f},
                                    {1.0f, 1.0f, 0.0f, 1.0f},
                                    {0.8f, 0.8f, 0.8f, 1.0f},
                                    {0.5f, 0.5f, 1.0f, 1.0f} };

    if(material <= 8) {
        color = palette[int(material)];
    } else {
        color = glm::vec4((float)material / 256.0f, 0.0f, 0.0f, 1.0f);
    }

    return color;
}

// main entry :)
int main(int argc, char* argv[])
{
    std::string exec_path = getexepath(); // path to executable of this process
    float dt; // time-step [sec]
    float dx; // space-step [m]
    glm::u32vec3 sc_size; // total scene size [simulation units / elements] - for now, one field is supported
    bool drivers_shown = true; // true if drivers has to be rendered
    bool mat_shown[8]; // true if objects of this material has to be rendered
    bool vox_map_shown = false;
    int last_key = 0; // last key of F1..9

    std::vector<f3d::scanner*> scanners;
    std::vector<f3d::object3d*> drivers;
    std::vector<f3d::object3d*> models;
    std::vector<uint8_t> models_mat; // material of each model
    std::vector<f3d::material_map*> vox_maps;

    if (argc < 2)
    {
        std::cerr << "ERR: no input file.\n";
        std::cerr << "Using: GL [project_file.json]\n";
        exit(-1);
    }

    for(int i = 0; i < sizeof(mat_shown) / sizeof(bool); i++) {
        mat_shown[i] = true;
    }

    /*** Init OpenGL ***/
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "GL Wave Explorer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 1024, 768);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_PROGRAM_POINT_SIZE); // vertex shader can control point size
    glEnable(GL_DEPTH_TEST); // using z-buffer
    glEnable(GL_BLEND); // transparency enable
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ...
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    /*** init shaders ***/
    f3d::shader scanner_shader((exec_path + "/f3d/vertex_scanner.glsl").c_str(), (exec_path + "/f3d/fragment_scanner.glsl").c_str()); // common shader for all scanners
    f3d::shader grid_shader((exec_path + "/f3d/vertex_grid.glsl").c_str(), (exec_path + "/f3d/fragment.glsl").c_str()); // common grid shader
    f3d::shader object_shader((exec_path + "/f3d/vertex_object.glsl").c_str(), (exec_path + "/f3d/fragment_object.glsl").c_str()); // common shader for all objects except scanner and woxel maps
    f3d::shader voxel_shader((exec_path + "/f3d/vertex_mat_map.glsl").c_str(), (exec_path + "/f3d/fragment_mat_map.glsl").c_str()); // common shader for all voxel maps

    /*** Parse .json ***/
    try {
        int fcntr, cntr;
        json jproject, val;
        std::ifstream project_file;
        project_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        // try to open and parse input project-file
        project_file.open(argv[1]);
        jproject = json::parse(project_file);

        /*** overal project parameters ***/
        val = jproject["steps"];
        if(!val.is_number_unsigned()) throw std::runtime_error("A valid number of simulation steps was not specified");
        num_frames = val;
        if(!(val = jproject["dt"]).is_number()) throw std::runtime_error("A valid time-step (dt) was not specified");
        dt = val;
        if(!(val = jproject["dx"]).is_number()) throw std::runtime_error("A valid space-step (dx) was not specified");
        dx = val;

        /*** load fields ***/
        fcntr = 0; // field index counter
        for(auto jf : jproject["fields"])
        {
            std::cout << "Creating new field: ";
            if((val = jf["name"]).is_string())
                std::cout << std::string(val);
            std::cout << "\n";
            try
            {
                sc_size = parse_vec3<double>(jf["size"]) * (1.0 / dx);
            }
            catch(const std::exception& e)
            {
                throw std::runtime_error("Field [" + std::to_string(fcntr) + "]: size is not specified:\n" + e.what());
            }

            /*** create drivers from .stl models ***/
            cntr = 0;
            for( auto jm : jf["drivers"] )
            {
                std::string path;

                // parse path
                if(!(val = jm["model"]).is_string())
                {
                    throw std::runtime_error("Field [" + std::to_string(fcntr) + "]: " \
                        "driver [" + std::to_string(cntr) + "]: \"model\" not specified\n");
                }
                path = val;
                auto o = new f3d::object3d(object_shader, f3d::loader::LoadSTL(path.c_str()),
                            {0, 0, 0},
                            {0, 0, 0},
                            {1.0/dx, 1.0/dx, 1.0/dx}, // use scale to convert from meters to simulation units
                            ColorFromMaterial(cntr + 1)); // driver's index used as "material" id for colouring
                drivers.push_back(o);
                cntr++;
            }

            /*** try to load voxel map (material map) ***/
            try {
                // one file for every field, created by FAS -> STL2VOX before simulation
                auto vm = new f3d::material_map(voxel_shader, sc_size, std::string("F" + std::to_string(fcntr) + "_drv.ui8").c_str());
                vox_maps.push_back(vm);
            }
            catch(const std::exception& e) {
                // nothing to do
            }

            /*** create scanners ***/
            cntr = 0;
            for( auto jscan : jf["scanners"] )
            {
                glm::u32vec3 position;
                glm::u32vec2 size;
                glm::vec3 rotation;
                std::string file_name;
                uint32_t store_every_nth_frame = 1;

                // parse position, size and rotation
                try
                {
                    position = parse_vec3<double>(jscan["position"]) * (1.0 / dx); // convert from meters to simulation-units
                    size = parse_vec2<double>(jscan["size"]) * (1.0 / dx);
                    rotation = parse_vec3<double>(jscan["rotation"]);
                }
                catch(const std::exception& e)
                {
                    throw std::runtime_error("Field [" + std::to_string(fcntr) + "]: " \
                        "scanner [" + std::to_string(cntr) + "]: position, size or rotation vector has invalid format");
                }
                // try to parse file_name
                if((val = jscan["out_file"]).is_string())
                {
                    file_name = val;
                }
                else
                {
                    // no file name specified, use some default
                    file_name = "f" + std::to_string(fcntr) + "s" + std::to_string(cntr) + "data.f32";
                }
                // try to parse how many frames to store (default is 1 - every frame)
                if((val = jscan["store_every_nth_frame"]).is_number_unsigned())
                {
                    store_every_nth_frame = val;
                    if(store_every_nth_frame < 1)
                        store_every_nth_frame = 1;
                }
                auto s = new f3d::scanner(scanner_shader, position, rotation, size, file_name, store_every_nth_frame);
                scanners.push_back(s);
                cntr++;
            }

            /*** create objects from .stl models ***/
            cntr = 0;
            for( auto jm : jf["models"] )
            {
                std::string path;
                uint8_t mat_id;

                // parse path
                if(!(val = jm["path"]).is_string())
                {
                    throw std::runtime_error("Field [" + std::to_string(fcntr) + "]: " \
                        "model [" + std::to_string(cntr) + "]: \"path\" not specified\n");
                }
                path = val;
                // parse material id
                if(!(val = jm["material_id"]).is_number_unsigned())
                {
                    std::cerr << "Field [" + std::to_string(fcntr) + "]: " \
                        "model [" + std::to_string(cntr) + "]: \"material_id\" not specified - assuming #0\n";
                    mat_id = 0;
                }
                else
                {
                    mat_id = val;
                    if(mat_id > 255)
                    {
                        std::cerr  << "Field [" + std::to_string(fcntr) + "]: " \
                            "model [" + std::to_string(cntr) + "]: \"material_id\" greater than 255 - #0 will be used instead\n";
                        mat_id = 0;
                    }
                }
                auto o = new f3d::object3d(object_shader, f3d::loader::LoadSTL(path.c_str()),
                            {0, 0, 0},
                            {0, 0, 0},
                            {1.0/dx, 1.0/dx, 1.0/dx}, // use scale to convert from meters to simulation units
                            ColorFromMaterial(mat_id));
                models.push_back(o);
                models_mat.push_back(mat_id);
                cntr++;
            }

            /*** try to load voxel map (material map) ***/
            try {
                // one file for every field, created by FAS -> STL2VOX before simulation
                auto vm = new f3d::material_map(voxel_shader, sc_size, std::string("F" + std::to_string(fcntr) + ".ui8").c_str());
                vox_maps.push_back(vm);
            }
            catch(const std::exception& e) {
                // nothing to do
            }

            fcntr++;
        }
    }
    catch(const std::exception& e) {
        std::cerr << "ERR: Preparing scene from file \"" << argv[1] << "\": " << e.what() << '\n';
        exit(-1);
    }
    
    // adjust move / frame-inc / rotate speed
    float scene_max_dim = std::max(std::max(sc_size.x, sc_size.y), sc_size.z); // maximal dimmension of scene
    float cameraSpeed = 0.35f * scene_max_dim; // adjust accordingly to scene size TODO: let user to adjust
    float rotationSpeed = M_PI * 0.5f; // 6.28 rad per 4 sec (by arrows on keyboard)

    f3d::grid grid1(grid_shader);
    grid1.Prepare(sc_size, {10.0f,10.0f,10.0f}); // todo: adjustable grid ?
    // TODO: multiply by "dx" to obtain [m] instead of simulation units
    std::cout << "Grid spacing [simulation units - dx]:\nx: " << 
        std::to_string(grid1.line_spacing.x) << "\ny: " <<
        std::to_string(grid1.line_spacing.y) << "\nz: " <<
        std::to_string(grid1.line_spacing.z) << "\n";

    int nbFrames = 0;
    float lastTime = 0.0f;
    float last_time = 0.0f;

    glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    unsigned int last_frame = 1;
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // "catch" mouse at center of window
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    auto io_thread = std::thread(cmd_input_thread);

    while (!glfwWindowShouldClose(window))
    {
        
        if (cmd_flag > 0)
        {
            if(cmd_line == "ge")
            {
                std::cout << "Last error: " << std::to_string(glGetError()) << "\n";
            }
            std::cout << cmd_line << std::endl; // TODO: parse & do command
            cmd_flag = 0;
        }
        else if (cmd_flag < 0)
        {
            // "quit" string or EOF entered to std input
            break;
        }
            
        // animation - camera
        float begin_of_frame_time = glfwGetTime();
        float delta_time = begin_of_frame_time - last_time;
        last_time = begin_of_frame_time;
        
        // inputs response
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rotate
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            cam_yaw -= delta_time * rotationSpeed;
        }
        else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            cam_yaw += delta_time * rotationSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            cam_pitch += delta_time * rotationSpeed;
            if (cam_pitch >= 0.49f * M_PI) cam_pitch = 0.49f * M_PI;
        }
        else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            cam_pitch -= delta_time * rotationSpeed;
            if (cam_pitch <= -0.49f * M_PI) cam_pitch = -0.49f * M_PI;
        }
        // move
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cam_pos += cameraSpeed * cam_front * delta_time;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cam_pos -= cameraSpeed * cam_front * delta_time;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * cameraSpeed * delta_time;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * cameraSpeed * delta_time;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            cam_pos += glm::normalize(glm::cross(glm::normalize(glm::cross(cam_front, cam_up)), cam_front)) * cameraSpeed * delta_time;
        }
        else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            cam_pos -= glm::normalize(glm::cross(glm::normalize(glm::cross(cam_front, cam_up)), cam_front)) * cameraSpeed * delta_time;
        }
        // wave animation - slow motion
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            if (frame > 0) frame--;
        }
        else if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            //if (frame < data.num_frames - 1) frame++;
            frame++;
        }
        // wave animation - fast motion
        if (glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS) {
            if (frame >= (1 + num_frames / 100))
                frame -= (1 + num_frames / 100);
        }
        else if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) {
            frame += (1 + num_frames / 100);
        }
        // show / hide maretials #1 .. #8 or voxel-map
        if(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F1)
                mat_shown[0] = !mat_shown[0];
            last_key = GLFW_KEY_F1;
        }
        else if(glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F2)
                mat_shown[1] = !mat_shown[1];
            last_key = GLFW_KEY_F2;
        }
        else if(glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F3)
                mat_shown[2] = !mat_shown[2];
            last_key = GLFW_KEY_F3;
        }
        else if(glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F4)
                mat_shown[3] = !mat_shown[3];
            last_key = GLFW_KEY_F4;
        }
        else if(glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F5)
                mat_shown[4] = !mat_shown[4];
            last_key = GLFW_KEY_F5;
        }
        else if(glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F6)
                mat_shown[5] = !mat_shown[5];
            last_key = GLFW_KEY_F6;
        }
        else if(glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F7)
                mat_shown[6] = !mat_shown[6];
            last_key = GLFW_KEY_F7;
        }
        else if(glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F8)
                mat_shown[7] = !mat_shown[7];
            last_key = GLFW_KEY_F8;
        }
        else if(glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F11)
                drivers_shown = !drivers_shown;
            last_key = GLFW_KEY_F11;
        }
        else if(glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS) {
            if(last_key != GLFW_KEY_F12)
                vox_map_shown = !vox_map_shown;
            last_key = GLFW_KEY_F12;
        } else {
            last_key = 0;
        }

        glm::vec3 direction;
        direction.x = cos(cam_yaw) * cos(cam_pitch);
        direction.y = sin(cam_pitch);
        direction.z = sin(cam_yaw) * cos(cam_pitch);
        cam_front = glm::normalize(direction);
        glm::mat4 camera = glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);
        glm::ivec4 viewport;
        glGetIntegerv(GL_VIEWPORT, &viewport.x); // get viewport position and size
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)viewport.z / viewport.w, 1.0f, 2.0f * scene_max_dim);
        camera = projection * camera;
        
        if (last_frame != frame)
        {
            // not same frame, load new values from files
            for( int i = 0; i < scanners.size(); i++ ) {
                scanners[i]->load_frame(frame);
            }
            last_frame = frame;
        }

        // render
        grid1.Draw(camera);
        for( int i = 0; i < scanners.size(); i++ ) {
            scanners[i]->Draw(camera);
        }
        if(vox_map_shown) {
            for( int i = 0; i < vox_maps.size(); i++ ) {
                vox_maps[i]->Draw(camera, cam_pos);
            }
        } else {
            if(drivers_shown) {
                for( int i = 0; i < drivers.size(); i++) {
                    drivers[i]->Draw(camera, cam_pos);
                }
            }
            for( int i = 0; i < models.size(); i++ ) {
                uint8_t mat = models_mat[i];
                if(mat == 0) {
                    continue;
                }
                if(mat <= 8) {
                    if(mat_shown[--mat]) {
                        models[i]->Draw(camera, cam_pos);
                    }
                }
                else {
                    models[i]->Draw(camera, cam_pos); // for now: models with mat# > 8 always shown
                }
            }
        }

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // FPS limiter - power save
        float now = glfwGetTime();
        if (now - begin_of_frame_time < frame_time) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(1e3f * (frame_time - now + begin_of_frame_time))));
        }

        // some usefull info
        // TODO: move it to gfx window
        printf("FPS: %f\n\r", 1.0f / (glfwGetTime() - begin_of_frame_time));
        printf("Frame: %u \033[F", frame);
    }

    glfwTerminate();
    std::exit(0); // io_thread will be forced to end here
    return 0;
}
