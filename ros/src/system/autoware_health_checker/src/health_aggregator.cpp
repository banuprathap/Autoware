#include <autoware_health_checker/health_aggregator.h>

HealthAggregator::HealthAggregator(ros::NodeHandle nh,ros::NodeHandle pnh)
{
    nh_ = nh;
    pnh_ = pnh;
}

HealthAggregator::~HealthAggregator()
{

}

void HealthAggregator::run()
{
    system_status_pub_ = nh_.advertise<autoware_system_msgs::SystemStatus>("/system_status",10);
    node_status_sub_ = nh_.subscribe("/node_status",10,&HealthAggregator::nodeStatusCallback,this);
    diagnostic_array_sub_ = nh_.subscribe("/diagnostic_agg",10,&HealthAggregator::diagnosticArrayCallback,this);
    return;
}

void HealthAggregator::publishSystemStatus()
{
    ros::Rate rate = ros::Rate(autoware_health_checker::UPDATE_RATE);
    while(ros::ok())
    {
        mtx_.lock();
        system_status_.header.stamp = ros::Time::now();
        system_status_pub_.publish(system_status_);
        system_status_.node_status.clear();
        system_status_.hardware_status.clear();
        mtx_.unlock();
        rate.sleep();
    }
    return;
}

void HealthAggregator::nodeStatusCallback(const autoware_system_msgs::NodeStatus::ConstPtr msg)
{
    mtx_.lock();
    system_status_.node_status.push_back(*msg);
    mtx_.unlock();
    return;
}

void HealthAggregator::diagnosticArrayCallback(const diagnostic_msgs::DiagnosticArray::ConstPtr msg)
{
    mtx_.lock();
    boost::optional<autoware_system_msgs::HardwareStatus> status = convert(msg);
    if(status)
    {
        system_status_.hardware_status.push_back(*status);
    }
    mtx_.unlock();
    return;
}

boost::optional<autoware_system_msgs::HardwareStatus> HealthAggregator::convert(const diagnostic_msgs::DiagnosticArray::ConstPtr msg)
{
    autoware_system_msgs::HardwareStatus status;
    if(msg->status.size() == 0)
    {
        return boost::none;
    }
    status.header = msg->header;
    for(auto diag_itr = msg->status.begin(); diag_itr != msg->status.end(); diag_itr++)
    {
        status.hardware_name = diag_itr->hardware_id;
        autoware_system_msgs::DiagnosticStatus diag;
        autoware_system_msgs::DiagnosticStatusArray diag_array;
        diag.header = msg->header;
        diag.key = diag_itr->hardware_id;
        diag.description = diag_itr->message;
        diag.type = autoware_system_msgs::DiagnosticStatus::HARDWARE;
        if(diag_itr->level == diagnostic_msgs::DiagnosticStatus::OK)
        {
            diag.level = autoware_health_checker::LEVEL_OK;
        }
        else if(diag_itr->level == diagnostic_msgs::DiagnosticStatus::WARN)
        {
            diag.level = autoware_health_checker::LEVEL_WARN;
        }
        else if(diag_itr->level == diagnostic_msgs::DiagnosticStatus::ERROR)
        {
            diag.level = autoware_health_checker::LEVEL_ERROR;
        }
        else if(diag_itr->level == diagnostic_msgs::DiagnosticStatus::STALE)
        {
            diag.level = autoware_health_checker::LEVEL_FATAL;
        }
        using namespace boost::property_tree;
        std::stringstream ss;
        ptree pt;
        for(auto value_itr = diag_itr->values.begin(); value_itr != diag_itr->values.end(); value_itr++)
        {
            pt.put(value_itr->key+".string", value_itr->value);
        }
        write_json(ss, pt);
        diag.value = ss.str();
        diag_array.status.push_back(diag);
        status.status.push_back(diag_array);
    }
    return status;
}