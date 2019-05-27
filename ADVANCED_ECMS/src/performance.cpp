#include <performance.hpp>
#include <VehicleInfo.hpp>
#include <ICEMG.hpp>
#include <data_loader.hpp>

using namespace LBK;
using namespace std;

std::vector<float> ECMS_performance::velocity_cycle = data_loader::get_1d_data("src/ADVANCED_ECMS/data/velocity_cycle");
std::vector<float> ECMS_performance::accel_cycle    = data_loader::get_1d_data("src/ADVANCED_ECMS/data/accel_cycle");

ECMS_performance::ECMS_performance(){}

void ECMS_performance::do_performance(Optimizer & obj_optimizer, string method, float time)
{
	ros::Time start = ros::Time::now();
	VehicleInfo::time_rt = time;
	float soc_t0 = MG::SOC;

	#pragma omp pararrel for
	for (register int ind=0; ind < accel_cycle.size(); ind++)
	{	
		VehicleInfo::accel = ECMS_performance::accel_cycle[ind];
		VehicleInfo::velocity = ECMS_performance::velocity_cycle[ind];

		int num = 0;
		bool update = false;
		while(!update)
		{
			update = Tool::ICEMG_parameter_update();
			num += 1;
		}

        // current soc
        ECMS_performance::SOC_per.push_back(MG::SOC); 

		obj_optimizer.optimizer(method);
        obj_optimizer.En_FC_rt(false);

        // optimal mode
        ECMS_performance::mode_per.push_back(int((std::get<0>(obj_optimizer.optimal_inform) == "HEV" ? 2 : 1) )
      + int((std::get<0>(obj_optimizer.optimal_inform) == "EV_Stop" ?  -1 : 0))
      + int((std::get<0>(obj_optimizer.optimal_inform) == "EV_Regen" ?  -2 : 0)));

        // optimal gear
        ECMS_performance::gear_per.push_back(std::get<1>(obj_optimizer.optimal_inform));
        // optimal dsoc
        ECMS_performance::dSOC_per.push_back(obj_optimizer.correction); 
        // optimal mass
        ECMS_performance::mass_per.push_back(obj_optimizer.En_FC_sum);
        // optimal lambda
        ECMS_performance::lambda_per.push_back(obj_optimizer.lambda); 
        // optimal mu
        ECMS_performance::mu_per.push_back(obj_optimizer.mu); 
        // optimal raw
        ECMS_performance::raw_per.push_back(obj_optimizer.raw);
	}
	ros::Time end = ros::Time::now();

	assert( abs(MG::SOC-soc_t0) <= pow(10, -2) && "Caution : SOC constraints is not zero!");

    // print
    obj_optimizer.En_FC_rt(true);
	cout << "Processing Time : " << (end - start).toSec() <<",  dSOC (soc tf - soc t0): " << MG::SOC-soc_t0 << endl;
}


void ECMS_performance::csvwrite(string method)
{
    ofstream myFile;
    myFile.open("src/ADVANCED_ECMS/output/dSOC_per.csv");

    for(register int ind = 0; ind < ECMS_performance::dSOC_per.size(); ind ++)
    {
        myFile << ind * VehicleInfo::time_rt <<
        ","<<ECMS_performance::dSOC_per[ind]<< endl;
    }
    
    myFile.close();
    myFile.open("src/ADVANCED_ECMS/output/SOC_per.csv");

    for(register int ind = 0; ind < ECMS_performance::SOC_per.size(); ind ++)
    {
        myFile << ind * VehicleInfo::time_rt <<
        ","<<ECMS_performance::SOC_per[ind]<< endl;
    }

    myFile.close();
    myFile.open("src/ADVANCED_ECMS/output/gear_per.csv");

    for(register int ind = 0; ind < ECMS_performance::gear_per.size(); ind ++)
    {
        myFile << ind * VehicleInfo::time_rt <<
        ","<<ECMS_performance::gear_per[ind]<< endl;
    }

    myFile.close();
    myFile.open("src/ADVANCED_ECMS/output/mode_per.csv");

    for(register int ind = 0; ind < ECMS_performance::mode_per.size(); ind ++)
    {
        myFile << ind * VehicleInfo::time_rt <<
        ","<<ECMS_performance::mode_per[ind]<< endl;
    }

    myFile.close();
    myFile.open("src/ADVANCED_ECMS/output/mass_per.csv");

    for(register int ind = 0; ind < ECMS_performance::mass_per.size(); ind ++)
    {
        myFile << ind * VehicleInfo::time_rt <<
        ","<<ECMS_performance::mass_per[ind]<< endl;
    }
    myFile.close();


    if(method == "L")
    {
        myFile.open("src/ADVANCED_ECMS/output/lambda_per.csv");

        for(register int ind = 0; ind < ECMS_performance::lambda_per.size(); ind ++)
        {
            myFile << ind * VehicleInfo::time_rt <<
            ","<<ECMS_performance::lambda_per[ind]<< endl;
        }

        myFile.close();
    }
    else
    {
        myFile.open("src/ADVANCED_ECMS/output/mu_per.csv");

        for(register int ind = 0; ind < ECMS_performance::mu_per.size(); ind ++)
        {
            myFile << ind * VehicleInfo::time_rt <<
            ","<<ECMS_performance::mu_per[ind]<< endl;
        }

        myFile.close();

        myFile.open("src/ADVANCED_ECMS/output/raw_per.csv");

        for(register int ind = 0; ind < ECMS_performance::raw_per.size(); ind ++)
        {
            myFile << ind * VehicleInfo::time_rt <<
            ","<<ECMS_performance::raw_per[ind]<< endl;
        }

        myFile.close();
    }
}