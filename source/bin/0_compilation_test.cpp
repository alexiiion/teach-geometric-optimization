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


    bool callback_update_view(igl::opengl::glfw::Viewer& viewer)
    {
        viewer.data().clear();
        viewer.data().set_mesh(V, F);

        /* 
        *  TODO: make necessary updates
        */


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
            /* TODO: add our interaction:
            *  (1) center mesh (also add coordinate indicator)
            *  (2) scale mesh
            */ 
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

        viewer.core().background_color.setOnes();
        viewer.core().set_rotation_type(igl::opengl::ViewerCore::ROTATION_TYPE_TRACKBALL);
        viewer.resize(1600, 1400);

        viewer_register_callbacks();
    }


    int inner_main(int argc, char* argv[])
    {
        // Initialize the viewer
        initialize_view();

        // Load a mesh in OBJ format
        igl::readOBJ(PathHelper::get_folder_path(__FILE__) + "/../../models/bunny.obj", V, F);
        //igl::readOBJ(MODEL_PATH "bunny_uniform.obj", V, F);

        viewer.launch();
    }
}
