#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>

#include "common/common.h"
#include "common/imgui.h"

#include "ruffle/ruffle.h"
#include "simulation/verlet.h"
#include "simulation/lbfgs.h"
#include "simulation/combination.h"

#include "visualization/visualization.h"

#include <unordered_map>

#include <chrono>

namespace ruffles {
	int inner_main(int argc, char* argv[]);
}
int main(int argc, char* argv[]) {
	try {
		return ruffles::inner_main(argc, argv);
	}
	catch (char const* x) {
		std::cerr << "Error: " << std::string(x) << std::endl;
	}
	return 1;
}


namespace ruffles {

	igl::opengl::glfw::Viewer viewer;
	igl::opengl::glfw::imgui::ImGuiMenu menu;

	using simulation::SimulationMesh;
	using visualization::Visualization;

	Ruffle ruffle;
	Visualization vis;

	real f = 0.;
	real f_prev = 0.;
	real converge_threshold = 1e-1;

	real y_target = 0.;
	real y_current = 0.;
	real y_target_step = 0.;
	real y_step = 0.5;

	bool show_forces = true;
	bool is_simulating = false;


	bool has_step_converged()
	{
		return abs(y_target_step - y_current) < converge_threshold && abs(f - f_prev) < converge_threshold;
	}

	bool has_converged()
	{
		return has_step_converged() && y_target_step <= y_target;
	}

	real get_displacement()
	{
		real y = 0.;
		for (auto v : ruffle.simulation_mesh.vertices) {
			y = max(y, ruffle.simulation_mesh.get_vertex_position(v)(1));
		}

		return y;
	}

	void set_target_displacement(double offset)
	{
		y_step = offset;
		y_target += y_step;
		y_target_step = y_target;
		ruffle.simulation_mesh.ub(1) = y_target_step;
	}

	void update_force() 
	{
		f_prev = f;
		f = 0.;

		VectorX grad = VectorX::Zero(ruffle.simulation_mesh.dof());
		ruffle.simulation_mesh.energy(ruffle.simulation_mesh.x, &grad);

		for (int i = 0; i < ruffle.simulation_mesh.dof(); i += 2) {
			Vector2 pos = ruffle.simulation_mesh.x.segment<2>(i);
			
			if (pos(1) > y_target - 1e-3) {
				if (grad(i + 1) > 0.) {
					cout << "    grad(i + 1) > 0." << endl; 
				}
				else {
					f += -grad(i + 1);
				}
			}
		}
	}

	void update_visualization() 
	{
		y_current = get_displacement();
		vis.clear();

		auto& mesh = ruffle.simulation_mesh;
		std::unordered_map<listref<SimulationMesh::Vertex>, int, listref_hash<SimulationMesh::Vertex>> indices;

		int i = 0;
		for (auto it = mesh.vertices.begin(); it != mesh.vertices.end(); ++it) {
			Vector2 pos = mesh.get_vertex_position(*it);
			vis.push_vertex((Vector3() << pos, 0.).finished());
			vis.push_vertex((Vector3() << pos, 4.).finished());
			indices.insert({ it, i });
			i++;
		}

		for (auto seg : mesh.segments) {
			int a = indices[seg.start];
			int b = indices[seg.end];

			vis.push_face(2 * a, 2 * b, 2 * b + 1);
			vis.push_face(2 * b + 1, 2 * a + 1, 2 * a);
		}
		vis.compress();

		viewer.data().clear();
		viewer.data().clear_labels();

		if (show_forces) {
			MatrixX p1(mesh.dof() / 2, 2);
			MatrixX p2(mesh.dof() / 2, 2);

			VectorX grad = VectorX::Zero(mesh.dof());
			mesh.energy(mesh.x, &grad);
			VectorX y = mesh.x - 0.003 * grad / mesh.k_global;
			for (int i = 0; i < mesh.dof(); i += 2) {
				p1.row(i / 2) << mesh.x.segment<2>(i).transpose();
				p2.row(i / 2) << y.segment<2>(i).transpose();
			}
			viewer.data().add_edges(p1, p2, Eigen::RowVector3d(0.8, 0., 0.));
		}

		i = 0;
		for (auto it = mesh.vertices.begin(); it != mesh.vertices.end(); ++it) {
			Vector2 pos = mesh.get_vertex_position(*it);
			viewer.data().add_label((Vector3() << pos, 0.).finished().cast<double>(), to_string(i));
			i++;
		}

		viewer.data().set_mesh(vis.V.cast<double>(), vis.F);
		viewer.data().uniform_colors(Vector3(0.3, 0.3, 0.3), Vector3(207.0 / 255.0, 233.0 / 255.0, 243.0 / 255.0), Vector3(0, 0, 0));

		viewer.data().double_sided = true;
		viewer.data().face_based = true;
	}

	bool callback_update_view(igl::opengl::glfw::Viewer& viewer)
	{
		if (!is_simulating)
			return false;

		if (!has_step_converged())
		{
			ruffle.physics_solve();
			update_force();
			update_visualization();
		}
		else if (!has_converged())
		{
			y_target_step -= y_step;
			ruffle.simulation_mesh.ub(1) = y_target_step;

			ruffle.physics_solve();
			update_force();
			update_visualization();
		}

		return false;
	}

	void callback_update_menu()
	{
		if (ImGui::CollapsingHeader("Mesh View"))
		{
			menu.draw_viewer_menu();
		}

		if (ImGui::CollapsingHeader("Ruffle", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("show forces", &show_forces);

			if (ImGui::InputReal("target height", &y_target)) {
				y_target_step = y_current - y_step;
				ruffle.simulation_mesh.ub(1) = y_target_step;
				//ruffle.simulation_mesh.ub(1) = y_target;
				update_visualization();
			}

			ImGui::Text("Apply displacement");
			if (ImGui::Button("down")) {
				set_target_displacement(-abs(y_step));
				update_visualization();
			}
			ImGui::SameLine();
			if (ImGui::Button("up")) {
				set_target_displacement(abs(y_step));
				update_visualization();
			}
			if (ImGui::Button("Solve"))
			{
				is_simulating = !is_simulating;
			}

			if (ImGui::Button("Solve one Step")) {
				is_simulating = false;

				ruffle.physics_solve();
				update_force();
				update_visualization();
			}


			ImGui::Text("f = %f N", f * 1e-4 / ruffle.simulation_mesh.k_global);
			ImGui::Text("y_target = %f", y_target);
			ImGui::Text("y_current = %f", y_current);
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
		//TODO add different ruffles

		//Ruffle ruffle = Ruffle::create_ruffle_stack(4, 0.5, 1.0, 0.1);
		//Ruffle ruffle = Ruffle::create_horizontal_stack(4, 4., 3., 0.5);
		
		// wide stack:
		//Ruffle ruffle = Ruffle::create_ruffle_stack(2, 3., 10.67, 0.5);
		
		// default stack
		ruffle = Ruffle::create_ruffle_stack(2, 3., 5.28, 0.5);
		ruffle.simulator.reset(new simulation::Verlet(ruffle.simulation_mesh));
		//ruffle.simulator.reset(new simulation::Combination(ruffle.simulation_mesh));
		ruffle.simulation_mesh.density = 0.160; // 160g paper
		ruffle.simulation_mesh.k_bend = 105000; // 160g paper  //TODO check this!!
		ruffle.simulation_mesh.update_vertex_mass();

		ruffle.update_simulation_mesh();

		for (int i = 0; i < 10; i++)
			ruffle.physics_solve();
		
		ruffle.simulation_mesh.generate_air_mesh(); //TODO add this to visualization & update!
		for (int i = 0; i < 10; i++)
			ruffle.physics_solve();

		y_current = get_displacement();
		y_target = y_current;
		y_target_step = y_target;
		ruffle.simulation_mesh.ub(1) = y_target;
		
		update_force();
		update_visualization();


		initialize_view();
		viewer.launch();
		return 0;
	}
}
