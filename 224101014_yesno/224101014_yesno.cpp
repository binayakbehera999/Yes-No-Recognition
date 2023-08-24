// 224101014_yesno.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>

/* Global Variables
DC_Shift stores the fluctions of amplitude due to leakage of current
max_Amp stores the max amplitude for normalisation
*/
long double DC_Shift = 0, max_Amp = 0;

// flags to classify yes or no
int flag_Yes = 1, flag_No = 1;

// Structure of link list
struct Node {
	long int fno; 
	long double energy;
	long int zcr;
	struct Node *next;
} *head;

typedef Node Node;

/*
@desc traverse the linked list
@param head of linked list
*/
void traverse(Node *head){
	Node *curr = head;
	while(curr){
		printf("%ld\t %Lf \t %ld\n", curr -> fno, curr -> energy, curr -> zcr);
		curr = curr -> next;
	}
}


// @desc filters the noise at the start of speech wave
void filter_Silence_At_Start(){
	Node *curr = head;
	while(curr){
		if(curr-> energy > 1000){
			head = curr;
			break;
		}
		curr = curr -> next;
	}
}


// @desc Calculate DC Shift for the input file
long double calculate_DCShift(FILE *fp){
	long double sum_Amp = 0, count = 0;
	int current;
	while(!feof(fp)){
		fscanf_s(fp, "%d", &current);
		sum_Amp = sum_Amp + abs(current);
		count++;
	}

	return (sum_Amp / count);
}

// @desc Find the Max Amplitude for Normalisation
long double find_Max_Amp(FILE *input){
	long int curr_Amp, Max = 0;
	while(!feof(input)){
		fscanf_s(input, "%d", &curr_Amp);
		if(abs(curr_Amp) > Max)	Max = abs(curr_Amp);
	}
	return Max;
}

// @desc print normalised amplitude data into output file
void normalise_Amplitude(FILE *input, FILE *output){
	
	long double curr_Amp, normalisation_Factor;
	normalisation_Factor = 5000/max_Amp;
	while(!feof(input)){
		fscanf_s(input, "%Lf", &curr_Amp);
		curr_Amp = curr_Amp * normalisation_Factor;
		fprintf(output, "%Lf\n", curr_Amp);
	}
}


//@desc calculate short term energy and zcr values and print in outfile file
void calculate_Energy_ZCR(FILE *input, FILE *output){
	long double curr_Amp;
	long int count = 0, sign, zcr = 0, fno = 1;
	long double energy_result = 0, sq_sum = 0;
	Node *curr_Node;

	//set sign of first element

	fscanf_s(input, "%Lf", &curr_Amp);
	fprintf(output, "Frame No., Energy, ZCR\n");
	if(curr_Amp > 0) sign = 1;
	if(curr_Amp < 0) sign = -1;
	if(curr_Amp == 0) sign = 0;

	fseek(input, 0, SEEK_SET);

	while(!feof(input)){

		while(count++ < 100){
			if(!feof(input)){

				fscanf_s(input, "%Lf", &curr_Amp); //read current element

				if(curr_Amp >= 0)
					sq_sum = sq_sum + ((curr_Amp - DC_Shift)*(curr_Amp - DC_Shift)); //sum of square of elements
				else
					sq_sum = sq_sum + ((curr_Amp + DC_Shift)*(curr_Amp + DC_Shift)); //sum of square of elements


				// Calculate ZCR
				if(curr_Amp > 0 && sign == -1) zcr++;
				if(curr_Amp < 0 && sign == 1) zcr++;

				if(curr_Amp > 0) sign = 1;
				else sign = -1;

			}else{
				break;
			}
		}
		//calculate energy
		energy_result = sq_sum/count;
		
		Node *newNode = (Node *)malloc((sizeof(Node)));
		newNode -> fno = fno;
		newNode -> energy = energy_result;
		newNode -> zcr = zcr;
		newNode -> next = NULL;
		if(!head) {
			head = newNode;
			curr_Node = newNode;
		}
		else {
			curr_Node -> next = newNode;
			curr_Node = newNode;
		}
		fprintf(output, "%ld, %Lf, %ld\n", fno, energy_result, zcr);

		fno++;
		energy_result = 0;
		sq_sum=0;
		count = 0;
		zcr = 0;
	}
}

/* FUNCTIONS TO CHECK YES */

//@desc returns pointer to Node containing max energy
Node* find_Max_Node(Node *head){
	Node *curr = head, *max_Node;
	long double max = 0;
	while(curr){
		if((curr -> energy) > max){
			max_Node = curr;
			max = curr -> energy;
		}
		curr = curr -> next;
	}
	return max_Node;
}

// @desc checks energy levels till max energy for yes
void check_Energy_Increase_Till_Max(Node *head, Node *max_Node){
	Node *curr = head;
	while(curr -> next != max_Node){
		if((curr -> energy) > (curr -> next -> energy)){
			flag_Yes = 0;
			return;
		}
		curr = curr -> next;
	}
}

// @desc checks zcr levels till max energy for yes
void check_Low_ZCR_At_Start(Node *head, Node *end){
	int diff, c=0;
	Node *curr = head;
	while(curr != end){
		diff = abs((curr -> zcr) - (curr -> next -> zcr));
		diff = diff / (curr -> zcr);
		diff = diff * 100;
		if(diff > 30) {
			c++;
			if(c > 2){
			flag_Yes = 0;
			return;
			}
		}
		curr = curr -> next;
	}
}

// @desc finds the point from where the sh sound starts
Node* find_Sh(Node *start, Node *max_Node){
	Node *curr = start;
	while(curr){
		if((curr -> zcr) > (2 * (max_Node -> zcr))) {
			return curr;
		}
		curr = curr -> next;
	}
	return NULL;
}

// @desc checks energy levels from max energy point till end for Yes
void check_Energy_At_End(Node *start, long double max){
	Node *curr = start;
	int count = 15;
	while(count-- > 0 && curr){
		if((curr -> energy) > 0.2 * max){
			flag_Yes = 0;
			return;
		}
		curr = curr -> next;
	}
	if(!curr) flag_Yes = 0;
}


// @desc checks zcr from max energy till end for yes
void check_ZCR_At_End(Node *start){
	int count = 15, diff, c=0;
	Node *curr = start;
	while(count-- && curr -> next){
		if((curr -> zcr) > (curr -> next -> zcr)){
			diff = (curr -> zcr) - (curr -> next -> zcr);
			diff = diff/(curr -> zcr);
			diff = diff * 100;
			if(diff > 3){
				c++;
				if(c > 2){
					flag_Yes = 0;
					return;
				}
			}
		}
		curr = curr -> next;
	}
}


//@desc checks whether it is yes
int check_For_Yes(Node *head){
	Node *max_Node, *end_Of_Ye, *start_Of_Sh;
	int count = 9;
	max_Node = find_Max_Node(head);
	end_Of_Ye = max_Node;

	while(count--){
		end_Of_Ye = end_Of_Ye -> next;
		if(!end_Of_Ye) {
		flag_Yes = 0;
		return 0;
		}
	}

	check_Energy_Increase_Till_Max(head, max_Node);

	if(!flag_Yes) return 0;

	check_Low_ZCR_At_Start(head, end_Of_Ye);

	if(!flag_Yes) return 0;

	start_Of_Sh = find_Sh(end_Of_Ye, max_Node);

	if(!start_Of_Sh){
		flag_Yes = 0;
		return 0;
	}

	check_Energy_At_End(start_Of_Sh, max_Node -> energy);

	if(!flag_Yes) return 0;

	check_ZCR_At_End(start_Of_Sh);

	if(!flag_Yes) return 0;

	return 1;
}


/* FUNCTIONS TO CHECK NO */

//@desc checks the zcr level for No 
void check_ZCR_For_No(Node *head){
	Node *curr = head;
	while(curr){
		if((curr -> zcr) > 10){
			flag_No = 0;
			return;
		}
		curr = curr -> next;
	}
}

// @desc checks energy levels from start till max energy point for No
void check_Energy_Till_Max(Node *head, Node *max_Node){
	Node *curr = head;
	long double diff;
	while(curr -> next != max_Node){
		if((curr -> energy) > (curr -> next -> energy)){
			diff = (curr -> energy) - (curr -> next -> energy);
			diff = diff/(curr -> energy);
			diff = diff * 100;
			if(diff > 25){
				flag_No = 0;
				return;
			}
		}
		curr = curr -> next;
	}
}

// @desc find the point till which energy falls drastically
Node* find_Decrease_Point(Node *start){
	Node *curr = start;
	long double limit;
	limit = 0.3 * (start -> energy);
	while(curr){
		if((curr -> energy) < limit){
			return curr;
		}
		curr = curr -> next;
	}
	return NULL;
}

// @desc checks energy levels from max energy point  till decrease point for No
void check_From_Max_Decrease_Point(Node *start, Node *end){
	Node *curr = start;
	long double diff = 0;
	while((curr -> next) != end){
		if((curr -> energy) < (curr -> next -> energy)){
			diff = (curr -> next -> energy) - (curr -> energy);
			diff = diff/(curr -> energy);
			diff = diff * 100;
			if(diff < 25){
				flag_No = 0;
				return;
			}
		}
		curr = curr -> next;
	}
}

//// @desc checks energy levels from decrease point till end for No
void check_from_Decrease_Point_To_End(Node *start){
	Node *curr = start;
	int count = 0;
	while(curr -> next){
		if((curr -> energy) < (curr -> next -> energy)){
			count++;
		}
		if(count > 4) {
			flag_No = 0;
			return;
		}
	}
}

//@desc checks for No
int check_For_No(Node *head){
	Node *max_Node, *decrease_Point;

	max_Node = find_Max_Node(head);

	check_ZCR_For_No(head);

	if(!flag_No) return 0;

	check_Energy_Till_Max(head, max_Node);

	if(!flag_Yes) return 0;

	decrease_Point = find_Decrease_Point(max_Node);

	check_From_Max_Decrease_Point(max_Node, decrease_Point);

	if(!flag_Yes) return 0;

	check_from_Decrease_Point_To_End(decrease_Point);

	if(!flag_Yes) return 0;

	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	FILE *DC_Shift_File, *test_case, *test_case_normalise, *op_csv;
	head = NULL;

	// open file containing data for DC Shift
	DC_Shift_File = fopen("DC_Shift.txt", "r");
	if(DC_Shift_File == NULL)
		printf("DC_Shift_File not opening");


	DC_Shift = calculate_DCShift(DC_Shift_File);

	// open file containing data of testcase
	test_case = fopen("no.txt", "r"); // enter input filename here
	if(test_case == NULL)
		printf("Test Case file not opening");

	max_Amp = find_Max_Amp(test_case); 
	
	fseek(test_case, 0, SEEK_SET);

	// open file to store normalised data
	test_case_normalise = fopen("no_normalise.txt", "w"); //enter output for normalised data filename here
	if(test_case == NULL)
		printf("Test Case Normalise file not opening");

	// excel file to store energy and zcr
	op_csv = fopen("output_no_normalise.csv", "w"); // enter excel filename to store normalise data
	if(op_csv == NULL)
		printf("File not opening excel");
	
	fseek(test_case, 0, SEEK_SET);
	normalise_Amplitude(test_case, test_case_normalise);

	fseek(test_case, 0, SEEK_SET);
	calculate_Energy_ZCR(test_case, op_csv);

	filter_Silence_At_Start();
	
	check_For_Yes(head);

	check_For_No(head);
	
	//Output
	if(flag_Yes) printf("Yes");
	else{
		if(flag_No) printf("No");
		else printf("Unable to recognise the word");
	}

	// close file
	fclose(DC_Shift_File);
	fclose(test_case);
	fclose(test_case_normalise);
	fclose(op_csv);
	return 0;
}

