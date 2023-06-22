#include <igl/barycenter.h>
#include <igl/cotmatrix.h>
#include <igl/doublearea.h>
#include <igl/grad.h>
#include <igl/jet.h>
#include <igl/massmatrix.h>
#include <igl/per_vertex_normals.h>
#include <igl/readDMAT.h>
#include <igl/readOFF.h>
#include <igl/repdiag.h>
#include <igl/opengl/glfw/Viewer.h>

#include <iostream>

#include "model_path.h"

namespace mesh_smoothing {
    int inner_main(int argc, char* argv[]);
}
int main(int argc, char* argv[]) {
    try {
        return mesh_smoothing::inner_main(argc, argv);
    }
    catch (char const* x) {
        std::cerr << "Error: " << std::string(x) << std::endl;
    }
    return 1;
}


namespace mesh_smoothing {

    igl::opengl::glfw::Viewer viewer;

    Eigen::MatrixXd V, U;
    Eigen::MatrixXi F;
    Eigen::SparseMatrix<double> L;

    Eigen::MatrixXd curve_V(102, 2);
    Eigen::MatrixXd curve_U(102, 2);
    bool isCurveVisible = false;
    bool isMeshVisible = false;


    void initialize_curve()
    {
        curve_V << 96.35, 175.26, 42.58, 90.48, 73.62, 185.96, 30.9, 95.11, 49.74, 193.72, 18.74, 98.23, 25.07, 198.42, 6.28, 99.8,
            0, 200, -6.28, 99.8, -25.07, 198.42, -18.74, 98.23, -49.74, 193.72, -30.9, 95.11, -73.62, 185.96, -42.58, 90.48, -96.35, 175.26, -53.58, 84.43,
            -117.56, 161.8, -63.74, 77.05, -136.91, 145.79, -72.9, 68.45, -154.1, 127.48, -80.9, 58.78, -168.87, 107.17, -87.63, 48.18, -180.97, 85.16,
            -92.98, 36.81, -190.21, 61.8, -96.86, 24.87, -196.46, 37.48, -99.21, 12, -199.42, 12.02, -100.18, -0.49, -199.42, -13, -99.21, -13,
            -196.46, -37.48, -96.86, -24.87, -190.21, -61.8, -92.98, -36.81, -180.97, -85.16, -87.63, -48.18, -168.87, -107.17, -80.9, -58.78,
            -154.1, -127.48, -72.9, -68.45, -136.91, -145.79, -63.74, -77.05, -117.56, -161.8, -53.58, -84.43, -96.35, -175.26, -42.58, -90.48,
            -73.62, -185.96, -30.9, -95.11, -49.74, -193.72, -18.74, -98.23, -25.07, -198.42, -6.28, -99.8, 0, -200, 0, -200, 6.28, -99.8, 25.07, -198.42,
            18.74, -98.23, 49.74, -193.72, 30.9, -95.11, 73.62, -185.96, 42.58, -90.48, 96.35, -175.26, 53.58, -84.43, 117.56, -161.8, 63.74, -77.05,
            136.91, -145.79, 72.9, -68.45, 154.1, -127.48, 80.9, -58.78, 168.87, -107.17, 87.63, -48.18, 180.97, -85.16, 92.98, -36.81, 190.21, -61.8,
            96.86, -24.87, 196.46, -37.48, 99.21, -12, 199.42, -12.02, 100.18, 0.49, 199.42, 13, 99.21, 13, 196.46, 37.48, 96.86, 24.87, 190.21, 61.8,
            92.98, 36.81, 180.97, 85.16, 87.63, 48.18, 168.87, 107.17, 80.9, 58.78, 154.1, 127.48, 72.9, 68.45, 136.91, 145.79, 63.74, 77.05, 117.56, 161.8,
            53.58, 84.43, 96.35, 175.26;
        
        curve_V = curve_V / 100;
        curve_U = curve_V;
    }

    void get_curve_edges(const Eigen::MatrixXd& curve, Eigen::MatrixXd& out_start, Eigen::MatrixXd& out_end)
    {
        if (!curve.rows())
            return;

        out_start.resize(curve.rows() - 1, curve.cols());
        out_end.resize(curve.rows() - 1, curve.cols());

        for (int j = 0; j < curve.rows() - 1; j++)
        {
            out_start.row(j) = curve.row(j);
            out_end.row(j) = curve.row(j + 1);
        }
    }

    Eigen::MatrixXd smooth_curve(const Eigen::MatrixXd& path, const int iterations, const double smooth_rate, const double inflate_rate)
    {        
        // only set geometry visible on first key stroke (mode switch)
        if (!isCurveVisible) {
            isCurveVisible = true;
            return path;
        }

        const int n = path.rows();
        Eigen::MatrixXd path_smoothed = path;

        for (int t = 0; t < iterations; t++)
        {
            for (int i = 1; i < n - 1; i++)
            {
                Eigen::RowVector2d vb = path_smoothed.row(i - 1) - path_smoothed.row(i);
                Eigen::RowVector2d vf = path_smoothed.row(i + 1) - path_smoothed.row(i);

                Eigen::RowVector2d Lp = vb * 0.5 + vf * 0.5;
                Eigen::RowVector2d point = path_smoothed.row(i) + smooth_rate * Lp;

                point = point + inflate_rate * Lp; // Taubin smooth

                path_smoothed.row(i) = point;
            }
        }

        return path_smoothed;
    }


    void update_curve_view()
    {
        if (!isCurveVisible)
            return;

        // Clear edges and points from view
        viewer.data().clear_edges();
        viewer.data().clear_points();

        // Set new edges for curve after smoothing
        Eigen::MatrixXd edges_start, edges_end;
        get_curve_edges(curve_U, edges_start, edges_end);
        viewer.data().add_edges(edges_start, edges_end, Eigen::RowVector3d(0.0, 1.0, 0.0));
        viewer.data().set_points(curve_U, Eigen::RowVector3d(0.0, 1.0, 0.0));
    }

    Eigen::MatrixXd smooth_mesh(Eigen::MatrixXd& V, Eigen::MatrixXi& F)
    {
        // only set geometry visible on first key stroke (mode switch)
        if (!isMeshVisible) {
            isMeshVisible = true;
            return V;
        }

        /*
        ** This code is from libigl's tutorial on Laplacian operators
        ** https://libigl.github.io/tutorial/#laplacian
        ** https://github.com/libigl/libigl/blob/main/tutorial/205_Laplacian/main.cpp
        */

        // Recompute just mass matrix on each step
        Eigen::SparseMatrix<double> M;
        igl::massmatrix(V, F, igl::MASSMATRIX_TYPE_BARYCENTRIC, M);

        // Solve (M-delta*L) U = M*U
        const auto& S = (M - 0.001 * L);
        Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> solver(S);
        assert(solver.info() == Eigen::Success);
        V = solver.solve(M * V).eval();

        /*
        // Compute centroid and subtract (also important for numerics)
        Eigen::VectorXd dblA;
        igl::doublearea(V, F, dblA);
        double area = 0.5 * dblA.sum();

        Eigen::MatrixXd BC;
        igl::barycenter(V, F, BC);
        Eigen::RowVector3d centroid(0, 0, 0);
        for (int i = 0; i < BC.rows(); i++)
        {
            centroid += 0.5 * dblA(i) / area * BC.row(i);
        }
        V.rowwise() -= centroid;

        // Normalize to unit surface area (important for numerics)
        V.array() /= sqrt(area);
        */
        return V;
    }

    void update_mesh_view()
    {
        if (!isMeshVisible)
            return;

        viewer.data().set_mesh(U, F);
        // Set new mesh positions for view, update normals, recenter
        //viewer.data().set_vertices(U);
        viewer.data().compute_normals();
        //viewer.core().align_camera_center(U, F);
    }


    bool callback_update_view(igl::opengl::glfw::Viewer& viewer)
    {
        return false;
    }

    bool callback_key_up(igl::opengl::glfw::Viewer& viewer, unsigned char key, int mod)
    {
        switch (key)
        {
        case 'r':
        case 'R':
            U = V;
            isMeshVisible = false;
            curve_U = curve_V;
            isCurveVisible = false;
            viewer.data().clear();
            break;
        case 'm':
        case 'M':
        {
            U = smooth_mesh(U, F);
            update_mesh_view();
            break;
        }
        case 'c':
        case 'C':
        {
            curve_U = smooth_curve(curve_U, 1, 0.5, -0.4);
            update_curve_view();
            break;
        }
        default:
            return false;
        }
        
        return true;
    }

    void viewer_register_callbacks()
    {
        viewer.callback_pre_draw = callback_update_view; // calls at each frame
        viewer.callback_key_up = callback_key_up;
        //menu.callback_draw_viewer_menu = callback_update_menu;

        viewer.core().is_animating = true;
        viewer.core().animation_max_fps = 30.;
    }

    void initialize_view()
    {
        //// Attach a menu plugin
        //viewer.plugins.push_back(&menu);

        //data_model.viewer.core().background_color = Eigen::Vector4f(1, 1, 1, 1);
        viewer.core().background_color.setOnes();
        viewer.core().set_rotation_type(igl::opengl::ViewerCore::ROTATION_TYPE_TRACKBALL);
        viewer.resize(1600, 1400);
        viewer.data().point_size = 7.0;
        viewer.data().line_width = 1.5;

        viewer_register_callbacks();
    }


    int inner_main(int argc, char* argv[])
    {
        // Initialize the viewer
        initialize_view();
        viewer_register_callbacks();
        initialize_curve();


        // Load a mesh in OFF format
        igl::readOFF(PathHelper::get_folder_path(__FILE__) + "/../../models/cow.off", V, F);
        //igl::readOBJ(PathHelper::get_folder_path(__FILE__) + "/../../models/bunny_uniform.obj", V, F);


        // Compute Laplace-Beltrami operator: #V by #V
        igl::cotmatrix(V, F, L);


        // Initialize smoothing with base mesh
        U = V;
        viewer.data().set_mesh(U, F);

        //// Use original normals as pseudo-colors
        //Eigen::MatrixXd N;
        //igl::per_vertex_normals(V, F, N);
        //Eigen::MatrixXd C = N.rowwise().normalized().array() * 0.5 + 0.5;
        //viewer.data().set_colors(C);




        std::cout << "Press [c] to smooth curve." << std::endl;
        std::cout << "Press [m] to smooth mesh." << std::endl;
        std::cout << "Press [r] to reset." << std::endl;
        return viewer.launch();
    }
}