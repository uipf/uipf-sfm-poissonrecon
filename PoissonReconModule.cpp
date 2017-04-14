/**
 * UIPF SfM PoissonRecon
 * Copyright (C) 2017 Carsten Brandt <mail@cebe.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>

#include <uipf/logging.hpp>
#include <uipf/util.hpp>

#include <boost/filesystem.hpp>

#include <uipf-sfm/data/PointCloud.hpp>

#define UIPF_MODULE_NAME "Poisson Surface Reconstruction"
#define UIPF_MODULE_ID "uipfsfm.mesh.poisson"
#define UIPF_MODULE_CLASS PoissonReconModule
#define UIPF_MODULE_CATEGORY "mesh"

#define UIPF_MODULE_INPUTS \
		{"points", uipf::DataDescription(uipfsfm::data::PointCloud::id(), "point cloud.")}

// TODO create mesh data structure to load data in uipf instead of just putting it into a file
//#define UIPF_MODULE_OUTPUTS \
//		{"points", uipf::DataDescription(uipfsfm::data::PointCloud::id(), "mesh.")}

#define UIPF_MODULE_PARAMS \
		{"result_file", uipf::ParamDescription("Name of the file to save the mesh in.")}, \
		{"depth", uipf::ParamDescription("This integer is the maximum depth of the tree that will be used for surface reconstruction. Default 8.", true)}, \
		{"pointWeight", uipf::ParamDescription("This floating point value specifies the importance that interpolation of the point samples is given in the formulation of the screened Poisson equation. Default 4.0.", true)}, \
		{"color", uipf::ParamDescription("If specified, the reconstruction code assumes that the input is equipped with colors, a good value would be 16. Default empty.", true)}, \
		{"density", uipf::ParamDescription("Enabling this flag tells the reconstructor to output the estimated depth values of the iso-surface vertices. Default false.", true)}

// TODO add more parameters

#include <uipf/Module.hpp>


// TODO configure this path with cmake
#define POISSON_BINARY "uipf-poisson-recon"

void PoissonReconModule::run() {

	using namespace std;
	using namespace uipf;
	using namespace uipfsfm::data;

	PointCloud::ptr pointCloud = getInputData<PointCloud>("points");

	// reading options
	string options;
	string depth = getParam<string>("depth", "");
	if (!depth.empty()) {
		options += " --depth " + depth;
	}
	string pointWeight = getParam<string>("pointWeight", "");
	if (!pointWeight.empty()) {
		options += " --pointWeight " + pointWeight;
	}
	string color = getParam<string>("color", "");
	if (!color.empty()) {
		options += " --color " + color;
	}
	bool density = getParam<bool>("density", false);
	if (density) {
		options += " --density";
	}

	string plyFile = "points.ply";
	string resultFile = getParam<string>("result_file", "");
	if (resultFile.empty()) {
		throw ErrorException("result file must not be empty.");
	}

	// TODO put this into a temporary directory
	ofstream plyFileS(plyFile);
	pointCloud->serialize(plyFileS, "");
	plyFileS.close();

	// run binary
	string cmd = (string(POISSON_BINARY)
	              + string(" --in ") + plyFile
	              + string(" --out ") + resultFile
	              + options);
	UIPF_LOG_INFO("Running command: ", cmd);
	int ret = system(cmd.c_str());

	if (ret != 0) {
		throw ErrorException("PoissonRecon exited non-zero.");
	}

	// TODO create mesh data structure to load data in uipf instead of just putting it into a file

	UIPF_LOG_INFO("Written file to ", resultFile);
}