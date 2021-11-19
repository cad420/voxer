#include <voxer/Data/Annotation.hpp>
#include <voxer/Data/StructuredGrid.hpp>
#include <set>
#include <map>

namespace voxer {

auto Annotation::has_hole_inside() -> bool { return coordinates.size() > 1; }

Annotation::Bbox getBbox(const std::vector<cv::Point>& contour, const std::array<int32_t, 2>& wh) {
	Annotation::Bbox bbox = { wh[0], wh[1], -1, -1 };
	for (auto& p : contour) {
		bbox[0] = std::min(bbox[0], p.x);
		bbox[1] = std::min(bbox[1], p.y);
		bbox[2] = std::max(bbox[2], p.x);
		bbox[3] = std::max(bbox[3], p.y);
	}
	bbox[0] -= 1;
	bbox[1] -= 1;
	bbox[2] += 1;
	bbox[3] += 1;
	if (bbox[0] < 0) bbox[0] = 0;
	if (bbox[1] < 0) bbox[1] = 0;
	if (bbox[2] >= wh[0]) bbox[2] = wh[0] - 1;
	if (bbox[3] >= wh[1]) bbox[3] = wh[1] - 1;
	return bbox;
}

void getMasksAnnotations(
	std::vector<Annotation>& annotations,
	std::shared_ptr<StructuredGrid> structuredg,
	std::map<uint8_t,std::string>& id2label
) {
	// if (masks.size() != labels.size()) {
        
    //     throw runtime_error("Error masks");
	// 	return;
	// }
    
    uint32_t x_dim=structuredg->info.dimensions[0],y_dim=structuredg->info.dimensions[1],z_dim=structuredg->info.dimensions[2];
	size_t sliceNum = z_dim;
	for (size_t i = 0; i < sliceNum; i++) {
        auto slice_i = structuredg->get_slice(StructuredGrid::Axis::Z,i);
		// const cv::Mat& mask = masks[i];
		// const set<unsigned short>& label = labels[i];
		uint32_t annotationId = 0;
		// vector<PolygonAnnotation> annotationsMat;
		for (auto& labelId : id2label) {
			cv::Mat mask_l = cv::Mat::zeros(slice_i.height,slice_i.width, CV_8UC1);
            bool haslabel=false;
			for (int row = 0; row < slice_i.height; row++) {
				for (int col = 0; col < slice_i.width; col++) {
					uint8_t data = slice_i.at(row, col);
					if (data == labelId.first) {
						mask_l.at<uchar>(row, col) = data;
                        haslabel=true;
					}
				}
			}

            if(!haslabel)
                continue;

			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
            //CV_RETR_TREE RETR_LIST
			findContours(mask_l, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE, cv::Point());
            int cnt=0;
			for (auto& contour : contours) {
                
                
                std::vector<Annotation::Point> tmpc;
                for (auto& point : contour) {
                    Annotation::Point point_A;
                    point_A[0]=point.x;
                    point_A[1]=point.y;
                    tmpc.emplace(tmpc.end(),std::move(point_A));
                }
                
                // std::copy(contour.begin(),contour.end(),tmpc.begin());

				auto bbox = getBbox(contour, { static_cast<int32_t>(slice_i.width), static_cast<int32_t>(slice_i.height)});

                if(hierarchy[cnt++][2]==-1)
                {
                    //outer
                }else
                {
                    //inner
                }
				Annotation annotation(bbox,std::string("polygon"),annotationId++, labelId.second,i, std::vector<std::decay_t<decltype(tmpc)>>{std::move(tmpc)});

				// annotationsMat.push_back(annotation);
		        annotations.push_back(std::move(annotation));
				// setAnnotationRingType(annotationsMat, annotationsMat.size() - 1);
			}
            
			// for (auto annotation : annotationsMat) {
			// 	set<size_t> innerRingIds;
			// 	for (auto idx : annotation.innerRingIds) {
			// 		innerRingIds.insert(annotationsMat[idx].id);
			// 	}
			// 	annotation.outerRingId = annotationsMat[annotation.outerRingId].id;
			// 	annotation.innerRingIds = innerRingIds;
			// }
		}
		// annotations.push_back(annotationsMat);
	}
}

std::shared_ptr<std::vector<Annotation>> getAnnotations(std::shared_ptr<StructuredGrid> structuredg,std::map<uint8_t,std::string>& id2label){
	// std::vector<cv::Mat> masks;
	std::vector<Annotation> annotations;

    // uint32_t x_dim=structuredg.info.dimensions[0],y_dim=structuredg.info.dimensions[1],z_dim=structuredg.info.dimensions[2];
    // for (int z = 0; z < z_dim; z++)
	// {
	// 	std::set<uint8_t> label;
	// 	cv::Mat img = cv::Mat::zeros(cv::Size(dim[1], dim[2]), CV_8UC1);
    //     size_t index = z * y_dim * x_dim;
	// 	for (int y = 0; y < y_dim; y++)
	// 	{
	// 		for (int x = 0; x < x_dim; x++)
	// 		{
	// 			uint8_t = data[index++];
	// 			if (f != 0) {
	// 				label.insert(f);
	// 			}
	// 			img.at<uchar>(y, x) = f;
	// 		}
	// 	}
	// 	masks.push_back(std::move(img));
	// 	labels.push_back(label);
	// }
    getMasksAnnotations(annotations, structuredg, id2label);
	return std::make_shared<std::vector<Annotation>>(std::move(annotations));
}

} // namespace voxer