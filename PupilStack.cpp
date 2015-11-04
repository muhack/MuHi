class CircleStack{

	float **hold;
	int size;
	float candidate[3];

	public:
		CircleStack(int _size){

			size = _size;

			hold = new float*[size];
			for (int h = 0; h < size; ++h)
				hold[h] = new float[3];

			//Init to all 0.0
			int i,j;
			for(i=0; i<size; i++){
				for(j=0; j<3; j++){
					hold[i][j]=0.0;
				}
			}

			for(i=0; i<3; i++)
				candidate[i]=0.0;
		}

	//private float *cvseqToFloat(CvSeq *input) { return 0.0; }

	//Update 3 value that describe the possible center
	// and radius of the most probable circle found
	void update_candidate(){

		float temp;
		int i,j;

		for(j=0; j<3; j++){
			temp=0;
			for(i=0; i<size; i++){
				temp=temp+hold[i][j];
			}
			//Update the corrisponding candidate's value
			candidate[j]=temp/size;
		}
	}

	float *get_candidate(){

		return candidate;

	}

	float *push(float *input){

		int i=0;
		for(i=0; i<size-1; i++){
			hold[i]=hold[i+1];
		}

		hold[size-1]=input;

		update_candidate();

		return input;
	}

	void empty_stack(){
		for(int i=0; i<size; i++)
			pop(i);
	}

	void pop(int index){

		if(index<=size && index>=0){
			hold[index][0]=0.0;
			hold[index][1]=0.0;
			hold[index][2]=0.0;
		}
	}

	float *get_element(int index){

		if(index<=size && index>=0)
			return hold[index];

		return NULL;
	}

	float *set_element(float *input, int index){

		if(index<=size && index>=0)
			hold[index]=input;
		else return NULL;

		return input;
	}
};
