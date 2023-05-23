#include "../include/printer_ik_solver.h"

PrinterIKSolver::PrinterIKSolver() :
tf_buffer_(),
tf_listener_(tf_buffer_),
move_group_(PLANNING_GROUP),
move_group_interface_(PLANNING_GROUP)
{
    ROS_INFO_STREAM("... ... ... ");
    /*Constructor*/
    moveit::core::RobotModelConstPtr robot_model = move_group_interface_.getRobotModel();
    moveit::core::RobotStatePtr robot_state(new moveit::core::RobotState(robot_model));
    move_group_interface_.setStartStateToCurrentState();

    const moveit::core::JointModelGroup* joint_model_group =
            move_group_interface_.getCurrentState()->getJointModelGroup(PLANNING_GROUP);

    robot_state_ = robot_state;
    joint_model_group_ = joint_model_group;

    ROS_INFO_STREAM("End effector: " << move_group_.getEndEffectorLink()); // lens_focal_work_frame
    ROS_INFO_STREAM("Reference frame: " << move_group_.getPoseReferenceFrame()); // base link
}

/*Calculate inverse kinematics for sPrinter robot lens, arguments:
 * desired pose in base_link frame and returned vector of joint values */
bool PrinterIKSolver::calculateIK(const geometry_msgs::PoseStamped& desired_pose, std::vector<double>& joint_values_ik)
{
    robot_state_->setToDefaultValues();
    bool ik_found = robot_state_->setFromIK(joint_model_group_, desired_pose.pose, 0.1);

    if (ik_found)
    {
         /*Get the resulting joint state values*/
        robot_state_->copyJointGroupPositions(joint_model_group_, joint_values_ik);

        ROS_INFO_STREAM("IK solution found");
        ROS_INFO_STREAM("Joint values: "
                        "\njoint 9 MainFrame_pitch: " << joint_values_ik[0] <<
                        "\njoint 10 Lens_Y_axis_trans: " << joint_values_ik[1] <<
                        "\njoint 11 Lens_X_axis_trans: " << joint_values_ik[2] <<
                        "\njoint 12 Lens_Y_axis_rot: " << joint_values_ik[3] <<
                        "\njoint 13 Lens_X_axis_rot: " << joint_values_ik[4]);
    }
    else
    {
        ROS_ERROR("Failed to compute IK solution");
        return false;
    }

    return true;
}

bool PrinterIKSolver::calculateIkService(sprinter_srvs::GetIkSolution::Request& req, sprinter_srvs::GetIkSolution::Response& res)
{
    ROS_INFO_STREAM("Called calculateIkService");
    try
    {
        calculateIK(req.pose, res.joint_states );
    }
    catch(...)
    {
        ROS_ERROR("ERROR when computing IK");
    }


    return true;
}