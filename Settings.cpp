//--------------------SETTINGS' CLASS-----------------------//

class Settings{

    public:
        float pixel_ratio;
        char *client_name;
        int pc_mode;
        char *main_win_title;
        int circleStackDim;
        int candidateTollerance;
        float applied_force; //KgF
        float sphere_diameter; //millimeter
    //Hough parameters

    //Blur parameters

    //Circle Find & Lock values

    char *get_default_setting_location(){

        return "settings.cfg";
    }

    //For every problem with setting, use this function
    //ATTENTION: maybe we must add "settings = new Settings();"
    void set_default_settings(){
    //----STATIC DEFAULT SETTINGS----
        float default_pixel_ratio = 1.0;
        char *default_client_name = "Barattieri SRL";
        int   default_pc_mode = 1;
        char *default_main_win_title = "Falcon 2.0";
        int   default_circleStackDim = 50;
        int   default_candidateTollerance = 5; //PERCENTAGE!!!
        float default_applied_force = 1; //KgF
        float default_sphere_diameter = 1; //millimeter
    //------------------------------

        pixel_ratio    = default_pixel_ratio;
        client_name    = default_client_name;
        pc_mode        = default_pc_mode;
        main_win_title = default_main_win_title;
        circleStackDim = default_circleStackDim;
        candidateTollerance = default_candidateTollerance;
        applied_force  = default_applied_force;
        sphere_diameter= default_sphere_diameter;
    }

    //Print out current settings
    void print_settings(){

        cout<<"----Current settings----"<<endl;
        cout<<"   Pixel ration: "<<pixel_ratio<<endl;
        cout<<"   Client's name: "<<client_name<<endl;
        cout<<"   PC mode: "<<pc_mode<<endl;
        cout<<"   Main windows title: "<<main_win_title<<endl;
        cout<<"   Circle Stack Dimension: "<<circleStackDim<<endl;
        cout<<"   Candidate Tollerance: "<<candidateTollerance<<"%"<<endl;
        cout<<"   Applied force: "<<applied_force<<"KgF"<<endl;
        cout<<"   Sphere Diameter: "<<sphere_diameter<<"mm"<<endl;
        cout<<"------------------------"<<endl;

    }

    //Funtion for reading applicatin settings - MUST WORK ON THIS
    //I don't like this code
    //Return 0 all went ok - retur 1 if there was an error
    int read_settings(char *file_path){

        string line;

        std::ifstream file(this->get_default_setting_location());

        if(file.is_open()){

            char *id;
            char *val;

            while(getline(file,line)){

                if(line.length()<=2) break; //EOF reach. I don't like this...

                char *cstr = new char[line.length() + 1];
                strcpy(cstr, line.c_str());

                id = strtok(cstr," :");
                val = strtok(NULL," :");

                if(strcmp(id,"pixel_ratio")==0)
                    sscanf(val,"%f",&pixel_ratio);

                if(strcmp(id,"client_name")==0){
                    client_name= (char*) malloc( (strlen(val)+1) * sizeof(char));
                    strcpy(client_name,val);
                }

                if(strcmp(id,"pc_mode")==0)
                    sscanf(val,"%i",&pc_mode);

                if(strcmp(id,"main_win_title")==0){
                    main_win_title= (char*) malloc( (strlen(val)+1) * sizeof(char));
                    strcpy(main_win_title,val);
                }

                if(strcmp(id,"circleStackDim")==0)
                    sscanf(val,"%i",&circleStackDim);

                if(strcmp(id,"candidateTollerance")==0)
                    sscanf(val,"%i",&candidateTollerance);

                if(strcmp(id,"applied_force")==0)
                    sscanf(val,"%f",&applied_force);

                if(strcmp(id,"sphere_diameter")==0)
                    sscanf(val,"%f",&sphere_diameter);
            }

        } else {
            file.close();
            return 1;
        }

        file.close();
        return 0;

    }

    //Function to write setting to file - MUST WORK ON THIS
    //I don't like this code, too
    //Return 0 all went ok - retur 1 if there was an error
    int write_settings(char *file_path){

        string line;

        std::ofstream file(get_default_setting_location());

        if(file.is_open()){

            file<<"pixel_ratio         : "<<pixel_ratio<<endl;
            file<<"client_name         : "<<client_name<<endl;
            file<<"pc_mode             : "<<pc_mode<<endl;
            file<<"main_win_title      : "<<main_win_title<<endl;
            file<<"circleStackDim      : "<<circleStackDim<<endl;
            file<<"candidateTollerance : "<<candidateTollerance<<endl;
            file<<"applied_force       : "<<applied_force<<endl;
            file<<"sphere_diameter     : "<<sphere_diameter<<endl;

        } else
            return 1;

        file.close();
        return 0;
    }
};
