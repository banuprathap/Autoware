/*
 *  Copyright (c) 2017, Tier IV, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name of Autoware nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIBNDT_SLAM_PCL_H
#define LIBNDT_SLAM_PCL_H

#include "points_localizer/ndt/ndt_slam_base.h"

#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl_registration/ndt.h>

template <class PointSource, class PointTarget>
class LibNdtSlamPCL
    : public LibNdtSlamBase<PointSource, PointTarget>
{
    public:
        LibNdtSlamPCL();
        ~LibNdtSlamPCL() = default;

        void setTransformationEpsilon(double trans_eps) override;
        void setStepSize(double step_size) override;
        void setResolution(float res) override;
        void setMaximumIterations(int max_iter) override;

        double getTransformationEpsilon() override;
        double getStepSize() const override;
        float getResolution() const override;
        int getMaximumIterations() override;
        double getTransformationProbability() const override;
        std::vector<Eigen::Vector3d> getCentroid() const override{
            return ndt_ptr_->getCentroid();
        };

        virtual std::vector<Eigen::Vector3d> getEval() const {
            return ndt_ptr_->getEval();
        };

        virtual std::vector<Eigen::Matrix3d> getEvec() const {
            return ndt_ptr_->getEvec();
        };

        std::vector<Eigen::Matrix3d> getCovariance() const override{
            return ndt_ptr_->getCovariance();
        };

    protected:
        void align(const Pose& predict_pose) override;
        double getFitnessScore() override;
        double getFitnessScore(const boost::shared_ptr< pcl::PointCloud<PointSource> const>& source_cloud, int* const nr, const double max_range) override;
        void setInputTarget(const boost::shared_ptr< pcl::PointCloud<PointTarget> const>& map_ptr) override;
        void setInputSource(const boost::shared_ptr< pcl::PointCloud<PointSource> const>& scan_ptr) override;
        Pose getFinalPose() override;

        void buildMap(const boost::shared_ptr< pcl::PointCloud<PointTarget> const>& map_ptr) override;
        void swapInstance() override;

    private:
        boost::shared_ptr< pcl::NormalDistributionsTransform<PointSource, PointTarget> > ndt_ptr_;
        boost::shared_ptr< pcl::NormalDistributionsTransform<PointSource, PointTarget> > swap_ndt_ptr_;
};

template <class PointSource, class PointTarget>
LibNdtSlamPCL<PointSource, PointTarget>::LibNdtSlamPCL()
    : ndt_ptr_(new pcl::NormalDistributionsTransform<PointSource, PointTarget>)
    , swap_ndt_ptr_(ndt_ptr_)
{
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::setTransformationEpsilon(double trans_eps)
{
    ndt_ptr_->setTransformationEpsilon(trans_eps);
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::setStepSize(double step_size)
{
    ndt_ptr_->setStepSize(step_size);
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::setResolution(float res)
{
    ndt_ptr_->setResolution(res);
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::setMaximumIterations(int max_iter)
{
    ndt_ptr_->setMaximumIterations(max_iter);
}

template <class PointSource, class PointTarget>
double LibNdtSlamPCL<PointSource, PointTarget>::getTransformationEpsilon()
{
    return ndt_ptr_->getTransformationEpsilon();
}

template <class PointSource, class PointTarget>
double LibNdtSlamPCL<PointSource, PointTarget>::getStepSize() const
{
    return ndt_ptr_->getStepSize();
}

template <class PointSource, class PointTarget>
float LibNdtSlamPCL<PointSource, PointTarget>::getResolution() const
{
    return ndt_ptr_->getResolution();
}

template <class PointSource, class PointTarget>
int LibNdtSlamPCL<PointSource, PointTarget>::getMaximumIterations()
{
    return ndt_ptr_->getMaximumIterations();
}

template <class PointSource, class PointTarget>
double LibNdtSlamPCL<PointSource, PointTarget>::getTransformationProbability() const
{
    return ndt_ptr_->getTransformationProbability();
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::align(const Pose& predict_pose)
{
    const auto predict_matrix = convertToEigenMatrix4f(predict_pose);
    pcl::PointCloud<PointSource> output_cloud;
    ndt_ptr_->align(output_cloud, predict_matrix);
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::setInputTarget(const boost::shared_ptr< pcl::PointCloud<PointTarget> const>& map_ptr)
{
    ndt_ptr_->setInputTarget(map_ptr);
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::setInputSource(const boost::shared_ptr< pcl::PointCloud<PointSource> const>& scan_ptr)
{
    ndt_ptr_->setInputSource(scan_ptr);
}

template <class PointSource, class PointTarget>
double LibNdtSlamPCL<PointSource, PointTarget>::getFitnessScore()
{
    return ndt_ptr_->getFitnessScore();
}

template <class PointSource, class PointTarget>
double LibNdtSlamPCL<PointSource, PointTarget>::getFitnessScore(const boost::shared_ptr< pcl::PointCloud<PointSource> const>& source_cloud, int* const nr, const double max_range)
{
    return ndt_ptr_->getFitnessScore(source_cloud, nr, max_range);
}

template <class PointSource, class PointTarget>
Pose LibNdtSlamPCL<PointSource, PointTarget>::getFinalPose()
{
    return convertToPose(ndt_ptr_->getFinalTransformation());
}

template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::buildMap(const boost::shared_ptr< pcl::PointCloud<PointTarget> const>& map_ptr)
{
    const auto trans_estimation = getTransformationEpsilon();
    const auto step_size = getStepSize();
    const auto resolution = getResolution();
    const auto max_iter = getMaximumIterations();

    boost::shared_ptr< pcl::NormalDistributionsTransform<PointSource, PointTarget> > tmp_ndt_ptr(new pcl::NormalDistributionsTransform<PointSource, PointTarget>);
    tmp_ndt_ptr->setTransformationEpsilon(trans_estimation);
    tmp_ndt_ptr->setStepSize(step_size);
    tmp_ndt_ptr->setResolution(resolution);
    tmp_ndt_ptr->setMaximumIterations(max_iter);
    tmp_ndt_ptr->setInputTarget(map_ptr);

    const auto identity_matrix = Eigen::Matrix4f::Identity();
    pcl::PointCloud<PointSource> output_cloud;
    tmp_ndt_ptr->align(output_cloud, identity_matrix);

    swap_ndt_ptr_ = tmp_ndt_ptr;
}


template <class PointSource, class PointTarget>
void LibNdtSlamPCL<PointSource, PointTarget>::swapInstance()
{
    ndt_ptr_ = swap_ndt_ptr_;
}

#endif
