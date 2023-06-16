#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

#include <iostream>

#include "model_path.h"


namespace compilation_test {
    int inner_main(int argc, char* argv[]);
}
int main(int argc, char* argv[]) {
    try {
        return compilation_test::inner_main(argc, argv);
    }
    catch (char const* x) {
        std::cerr << "Error: " << std::string(x) << std::endl;
    }
    return 1;
}


namespace compilation_test {

    igl::opengl::glfw::Viewer viewer;
    igl::opengl::glfw::imgui::ImGuiMenu menu;

    Eigen::MatrixXd V;
    Eigen::MatrixXi F;


    void center_mesh()
    {
        std::cout << "center mesh" << std::endl;

        //get centroid
        Eigen::RowVector3d min_point = V.colwise().minCoeff();
        Eigen::RowVector3d max_point = V.colwise().maxCoeff();
        Eigen::RowVector3d centroid = ((min_point + max_point) * 0.5).eval();

        V = V.rowwise() - centroid;
        std::cout << "translate by " << centroid << std::endl;
    }

    bool callback_update_view(igl::opengl::glfw::Viewer& viewer)
    {
        viewer.data().clear();

        Eigen::Matrix3d coord = Eigen::Matrix3d::Identity();
        viewer.data().add_edges(Eigen::Matrix3d::Zero(), coord, coord);

        viewer.data().set_mesh(V, F);
        viewer.data().add_label(viewer.data().V.row(0) + viewer.data().V_normals.row(0).normalized() * 0.005, "Hello World!");

        return false;
    }

    void callback_update_menu()
    {
        if (ImGui::CollapsingHeader("Mesh View"))
        {
            menu.draw_viewer_menu();
        }

        if (ImGui::CollapsingHeader("Our menu", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("center mesh"))
            {
                center_mesh();
            }
            if (ImGui::Button("2x mesh"))
            {
                V *= 2.0;
            }
        }
    }

    void viewer_register_callbacks()
    {
        viewer.callback_pre_draw = callback_update_view; // calls at each frame
        menu.callback_draw_viewer_menu = callback_update_menu;

        viewer.core().is_animating = true;
        viewer.core().animation_max_fps = 30.;
    }

    void initialize_view()
    {
        // Attach a menu plugin
        viewer.plugins.push_back(&menu);

        //data_model.viewer.core().background_color = Eigen::Vector4f(1, 1, 1, 1);
        viewer.core().background_color.setOnes();
        viewer.core().set_rotation_type(igl::opengl::ViewerCore::ROTATION_TYPE_TRACKBALL);
        viewer.resize(1600, 1400);

        viewer_register_callbacks();
    }


    int inner_main(int argc, char* argv[])
    {
        // Initialize the viewer
        initialize_view();
        viewer_register_callbacks();

        // Load a mesh in OBJ format
        //igl::readOBJ(MODEL_PATH "bunny.obj", V, F);

        igl::readOBJ(PathHelper::get_folder_path(__FILE__) + "/../../models/bunny.obj", V, F);
        //igl::readOBJ(MODEL_PATH "bunny_uniform.obj", V, F);



        viewer.launch();
    }
}

/*
igl::opengl::glfw::Viewer viewer;   // Init the viewer
igl::opengl::glfw::imgui::ImGuiMenu menu;   // Attach a menu plugin

Eigen::MatrixXd V;
Eigen::MatrixXi F;
double doubleVariable = 0.1f; // Shared between two menus


std::string get_folder_path(std::string file)
{
    auto index = file.find_last_of("/\\");
    auto path = file.substr(0, index + 1);
    return path;
}

std::string get_this_folder_path()
{
    return get_folder_path(__FILE__);
}

bool callback_update_view(igl::opengl::glfw::Viewer& viewer)
{

    viewer.data().set_mesh(V, F);
    viewer.data().add_label(viewer.data().V.row(0) + viewer.data().V_normals.row(0).normalized() * 0.005, "Hello World!");

    return false;
}

void callback_update_menu()
{
    //bool _viewer_menu_visible = true;
    //ImGui::Begin("Example", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    //ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

    // Draw parent menu content
    menu.draw_viewer_menu();

    // Add new group
    if (ImGui::CollapsingHeader("New Group", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Expose variable directly ...
        ImGui::InputDouble("double", &doubleVariable, 0, 0, "%.4f");

        // ... or using a custom callback
        static bool boolVariable = true;
        if (ImGui::Checkbox("bool", &boolVariable))
        {
            // do something
            std::cout << "boolVariable: " << std::boolalpha << boolVariable << std::endl;
        }

        // Expose an enumeration type
        enum Orientation { Up = 0, Down, Left, Right };
        static Orientation dir = Up;
        ImGui::Combo("Direction", (int*)(&dir), "Up\0Down\0Left\0Right\0\0");

        // We can also use a std::vector<std::string> defined dynamically
        static int num_choices = 3;
        static std::vector<std::string> choices;
        static int idx_choice = 0;
        if (ImGui::InputInt("Num letters", &num_choices))
        {
            num_choices = std::max(1, std::min(26, num_choices));
        }
        if (num_choices != (int)choices.size())
        {
            choices.resize(num_choices);
            for (int i = 0; i < num_choices; ++i)
                choices[i] = std::string(1, 'A' + i);
            if (idx_choice >= num_choices)
                idx_choice = num_choices - 1;
        }
        ImGui::Combo("Letter", &idx_choice, choices);

        // Add a button
        if (ImGui::Button("Print Hello", ImVec2(-1, 0)))
        {
            std::cout << "Hello\n";
        }
    }

    //ImGui::PopItemWidth();
    //ImGui::End();
}

void callback_update_custom_window()
{
    // Define next window position + size
    ImGui::SetNextWindowPos(ImVec2(180.f * menu.menu_scaling(), 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 160), ImGuiCond_FirstUseEver);
    ImGui::Begin(
        "New Window", nullptr,
        ImGuiWindowFlags_NoSavedSettings
    );

    // Expose the same variable directly ...
    ImGui::PushItemWidth(-80);
    ImGui::DragScalar("double", ImGuiDataType_Double, &doubleVariable, 0.1, 0, 0, "%.4f");
    ImGui::PopItemWidth();

    static std::string str = "bunny";
    ImGui::InputText("Name", str);

    ImGui::End();
}

void viewer_register_callbacks()
{
    viewer.callback_pre_draw = callback_update_view; // calls at each frame
    menu.callback_draw_viewer_menu = callback_update_menu;
    //menu.callback_draw_viewer_window = callback_update_menu;
    menu.callback_draw_custom_window = callback_update_custom_window;

    viewer.core().is_animating = true;
    viewer.core().animation_max_fps = 30.;
}

void initialize_view()
{
    // Attach a menu plugin
    viewer.plugins.push_back(&menu);

    //data_model.viewer.core().background_color = Eigen::Vector4f(1, 1, 1, 1);
    viewer.core().background_color << 1.0f, 1.0f, 1.0f, 1.0f;
    viewer.core().set_rotation_type(igl::opengl::ViewerCore::ROTATION_TYPE_TRACKBALL);
    viewer.resize(1600, 1400);
}

int main(int argc, char* argv[])
{
    // Load a mesh in OBJ format
    igl::readOBJ(get_this_folder_path() + "../../models/bunny.obj", V, F);

    // Initialize the viewer
    initialize_view();
    viewer_register_callbacks();

    viewer.launch();
}
*/