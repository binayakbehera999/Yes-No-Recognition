========================================================================
    CONSOLE APPLICATION : 224101014_yesno Project Overview
========================================================================

This is project to detect if the speech wave is Yes or No.

Device Model Name - ASUS-ROG-GL-503
The DC shift is calculated using device michrophone

There is one DC_Shift.txt to calculate DC_Shift.

Instructions to execute the code:
1. Enter the input filename at the test_case variable.
2. Enter name of output file txt at test_case_normalise variable.
3. Enter name of output excel file at op_csv variable.
4. Add a breakpoint before fclose.
5. Press F5.

Description of the Code:

1. Record a silence.txt with michrophone senstivity set to zero.
2. Calculate DC_Shift from silence.txt and subtract it with each data.
3. Find max amplitude of the input data.
4. normalisation factor = 5000/max amplitude.
5. Multiply every data with the normalisation factor.
6. Calculate ZCR and Energy levels of the input data.
7. Filter noises at start
8. Check for Yes
	i. Find max Energy Point
	ii. Check energy data is increasing till max point and zcr remains consistent
		(i.e. 30% variance) and low. If not make flag_Yes = 0;
	iii. Find the point where zcr rises drastically, denotes start of 'sh' sound.
		If no point exist make flag_Yes = 0.
	iv. From this point till end, check zcr is high and energy is low and consistent with
		a variance of 3%. If not make flag_Yes = 0;
9.Check for No
	i. Find max energy point.
	ii. check zcr level. It remains low and below a threshold.
		If it crosses threshold make flag_No = 0.
	iii. check energy rises till max point. If decreases at any point it is within 25%.
	iv. Find point after max energy point till which energy falls drastically.
	v. Check decrease of energy from max energy to decrease point. If increases at any
		point its within 25%.
	vi Check decrease of energy from decrease point till end.
10. Check if flag yes = 1. output - 'Yes'
    Check if flag no = 1. output - 'No'
////////////////////////////////////////////////////////////////////////////
