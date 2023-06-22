#include <igl/readOBJ.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>

#include <iostream>

#include "model_path.h"


namespace compilation_test__final {
	int inner_main(int argc, char* argv[]);
}
int main(int argc, char* argv[]) {
	try {
		return compilation_test__final::inner_main(argc, argv);
	}
	catch (char const* x) {
		std::cerr << "Error: " << std::string(x) << std::endl;
	}
	return 1;
}


namespace compilation_test__final {

    igl::opengl::glfw::Viewer viewer;
    igl::opengl::glfw::imgui::ImGuiMenu menu;

    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    
    
    
    
    // ****** NEW 
    double scale = 1.0;

    void draw_coordinate_indicator()
    {
        const auto coordinate_indicator = Eigen::MatrixXd::Identity(3, 3);
        viewer.data().add_edges(Eigen::MatrixXd::Zero(3, 3), coordinate_indicator * 0.2, coordinate_indicator);
    }

    Eigen::RowVector3d get_centroid(Eigen::MatrixXd& vertices)
    {
        const auto min_point = vertices.colwise().minCoeff();
        const auto max_point = vertices.colwise().maxCoeff();
        auto centroid = (0.5 * (min_point + max_point)).eval();

        return centroid;
    }

    void center_mesh()
    {
        Eigen::RowVector3d centroid = get_centroid(V);
        V = V.rowwise() - centroid;
        std::cout << "translate = " << centroid << std::endl;
    }

    void scale_mesh()
    {
        V *= scale;
        std::cout << "scaled mesh by " << scale << "x" << std::endl;
    }
    // ****** 



    bool callback_update_view(igl::opengl::glfw::Viewer& view)
    {
        viewer.data().clear();


        // ****** NEW 
        draw_coordinate_indicator();
        // ****** 


        viewer.data().set_mesh(V, F);

        if (viewer.data().show_custom_labels) {
            viewer.data().add_label(viewer.data().V.row(0) + viewer.data().V_normals.row(0).normalized() * 0.01, "V0");
            viewer.data().add_points(viewer.data().V.row(0), Eigen::MatrixXd::Zero(1, 3));
        }

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
            // ****** NEW 
            if (ImGui::Button("center mesh"))
                center_mesh();

            ImGui::InputDouble("scale factor", &scale);
            if (ImGui::Button("scale mesh"))
                scale_mesh();
            // ****** 
        }
    }

    void viewer_register_callbacks()
    {
        viewer.callback_pre_draw = callback_update_view; // calls at each frame
        menu.callback_draw_viewer_menu = callback_update_menu;

        viewer.core().is_animating = true;
        viewer.core().animation_max_fps = 30.;

        viewer.core().is_animating = true;
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
        //igl::readOBJ(PathHelper::get_folder_path(__FILE__) + "/../../models/bunny_uniform.obj", V, F);
        
        viewer.launch();
    }
}
