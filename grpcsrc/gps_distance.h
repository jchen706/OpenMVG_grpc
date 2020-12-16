// C++ program to calculate Distance 
// Between Two Points on Earth 
// geeksforgeeks reference
#include <bits/stdc++.h> 
using namespace std; 

// Utility function for 
// converting degrees to radians 
long double toRadians(const long double degree) 
{ 
	// cmath library in C++ 
	// defines the constant 
	// M_PI as the value of 
	// pi accurate to 1e-30 
	double one_deg = (M_PI) / 180; 
	return (one_deg * degree); 
} 

long double distance(double lat1, double long1, 
					double lat2, double long2) 
{ 
	// Convert the latitudes 
	// and longitudes 
	// from degree to radians. 
	lat1 = toRadians(lat1); 
	long1 = toRadians(long1); 
	lat2 = toRadians(lat2); 
	long2 = toRadians(long2); 
	
	// Haversine Formula 
	double dlong = long2 - long1; 
	double dlat = lat2 - lat1; 

	double ans = pow(sin(dlat / 2), 2) + 
						cos(lat1) * cos(lat2) * 
						pow(sin(dlong / 2), 2); 

	ans = 2 * asin(sqrt(ans)); 

	// Radius of Earth in 
	// Kilometers, R = 6371 
	// Use R = 3956 for miles 

    // for meters
	double R = 6371000; 
	
	// Calculate the result 
	ans = ans * R; 

	return ans; 
} 

// Driver Code 
// int main() 
// { 
// 	long double lat1 = 53.32055555555556; 
// 	long double long1 = -1.7297222222222221; 
// 	long double lat2 = 53.31861111111111; 
// 	long double long2 = -1.6997222222222223; 
	
// 	// call the distance function 
// 	cout << setprecision(15) << fixed; 
// 	cout << distance(lat1, long1, 
// 					lat2, long2) << " K.M"; 

// 	return 0; 
// } 

// This code is contributed 
// by Aayush Chaturvedi 
