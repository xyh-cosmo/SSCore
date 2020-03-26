//  SSIdentifier.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/20/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include "SSAngle.hpp"
#include "SSIdentifier.hpp"
#include <vector>
#include <map>

static vector<string> _bayvec =
{
	"alpha",
	"beta",
	"gamma",
	"delta",
	"epsilon",
	"zeta",
	"eta",
	"theta",
	"iota",
	"kappa",
	"lambda",
	"mu",
	"nu",
	"xi",
	"omicron",
	"pi",
	"rho",
	"sigma",
	"tau",
	"upsilon",
	"phi",
	"chi",
	"psi",
	"omega",
};

static vector<string> _convec =
{
	"And", "Ant", "Aps", "Aqr", "Aql", "Ara", "Ari", "Aur",
	"Boo", "Cae", "Cam", "Cnc", "CVn", "CMa", "CMi", "Cap",
	"Car", "Cas", "Cen", "Cep", "Cet", "Cha", "Cir", "Col",
	"Com", "CrA", "CrB", "Crv", "Crt", "Cru", "Cyg", "Del",
	"Dor", "Dra", "Equ", "Eri", "For", "Gem", "Gru", "Her",
	"Hor", "Hya", "Hyi", "Ind", "Lac", "Leo", "LMi", "Lep",
	"Lib", "Lup", "Lyn", "Lyr", "Men", "Mic", "Mon", "Mus",
	"Nor", "Oct", "Oph", "Ori", "Pav", "Peg", "Per", "Phe",
	"Pic", "Psc", "PsA", "Pup", "Pyx", "Ret", "Sge", "Sgr",
	"Sco", "Scl", "Sct", "Ser", "Sex", "Tau", "Tel", "Tri",
	"TrA", "Tuc", "UMa", "UMi", "Vel", "Vir", "Vol", "Vul"
};

static map<string,int> _conmap;
static map<string,int> _baymap;

static int string_to_bayer ( string str )
{
	size_t len = str.length();
	
	if ( len == 1 )
	{
		if ( str[0] >= 'a' && str[0] <= 'z' )
			return str[0] - 'a' + 25;
		else if ( str[0] >= 'A' && str[0] < 'R' )
			return str[0] - 'A' + 51;
	}
	else
	{
		for ( int i = 0; i < _bayvec.size(); i++ )
			if ( _bayvec[i].compare ( 0, len, str ) == 0 )
				return i + 1;
	}
	
	return 0;
}

static string bayer_to_string ( int64_t bay )
{
	if ( bay > 50 )
		return string ( 1, bay - 51 + 'A' );
	else if ( bay > 24 )
		return string ( 1, bay - 25 + 'a' );
	else
		return _bayvec[ bay - 1 ];
}

static int64_t string_to_gcvs ( string str )
{
	size_t len = str.length();
	int n1 = 0, n2 = 0;
	
	// Sequence R, S, T ... Z
		
	if ( len == 1 && str[0] >= 'R' && str[0] <= 'Z' )
	{
		return str[0] - 'R' + 1;
	}

	// Sequence RR, RS, RT ... SS, ST, SU, ... TT, TU ... ZZ
		
	else if ( len == 2 && str[0] >= 'R' && str[0] <= 'Z' && str[1] >= str[0] && str[1] <= 'Z' )
	{
		n1 = str[0] - 'R';
		n2 = str[1] - 'R';
		
		return n1 * 9 - ( n1 - 1 ) * n1 / 2 + ( n2 - n1 ) + 10;
	}
		
	// Sequence AA, AB, AC, ... BB, BC, BD, ... CC, CD, .... QZ
		
	else if ( len == 2 && str[0] >= 'A' && str[0] < 'R' && str[0] != 'J' && str[1] >= str[0] && str[1] <= 'Z' && str[1] != 'J'  )
	{
		n1 = str[0] - 'A';
		n2 = str[1] - 'A';
		
		// J is skipped!
		
		if ( str[0] >= 'K' )
			n1--;

		if ( str[1] >= 'K' )
			n2--;
		
		return n1 * 25 - ( n1 - 1 ) * n1 / 2 + ( n2 - n1 ) + 55;
	}

	// Sequence V335, V336, V337, V338, ...
		
	if ( len > 3 && str[0] == 'V' && str[1] >= '0' && str[1] <= '9' )
	{
		return strtoint ( str.substr ( 1, len - 1 ) );
	}
		
	return 0;
}

string gcvs_to_string ( int64_t n )
{
	int64_t n0 = 0, n1 = 0, n2 = 0;
	
	// Sequence R, S, T, ... Z
	
	if ( n < 10 )
	{
		return string ( 1, 'R' + n - 1 );
	}
	
	// Sequence RR, RS, RT, ... RZ, SS, ST, ... SZ, TT, TU, ... ZZ ***/
	
	else if ( n < 55 )
	{
		for ( n0 = n - 10, n1 = 0, n2 = 9; n2 <= n0; n1++, n2 += 9 - n1 )
			;
		n2 = n0 - n2 + 9;

		return string ( 1, 'R' + n1 ) + string ( 1, 'R' + n2 );
	}
	
	// Sequence AA, AB, AC, ... AZ, BB, BC, ... BZ, CC, CD, ... QZ
	
	else if ( n < 335 )
	{
		for ( n0 = n - 55, n1 = 0, n2 = 25; n2 <= n0; n1++, n2 += 25 - n1 )
			;
		n2 = n0 - n2 + 25;

		// J is skipped!
		
		if ( n1 >= 'J' - 'A' )
			n1++;
		
		if ( n2 >= 'J' - 'A' )
			n2++;
			
		return string ( 1, 'A' + n1 ) + string ( 1, 'A' + n2 );
	}
	
	// Sequence V335, V336, V337, V338, etc.
	
	else
	{
		return "V" + to_string ( n );
	}
	
	return "";
}

int64_t string_to_dm ( string str )
{
	char 	sign = 0, suffix = 0;
    int		zone = 0, num = 0;
	
	sscanf ( str.c_str(), "%c%d%d%c", &sign, &zone, &num, &suffix );

    if ( sign == '+' )
    	sign = 1;
    else
    	sign = 0;

    if ( suffix == 'a' || suffix == 'A' )
    	suffix = 1;
    else if ( suffix == 'b' || suffix == 'B' )
    	suffix = 2;
    else if ( suffix == 'n' || suffix == 'N' )
    	suffix = 3;
    else if ( suffix == 'p' || suffix == 'P' )
    	suffix = 4;
    else if ( suffix == 's' || suffix == 'S' )
    	suffix = 5;
	else
    	suffix = 0;
    
	return sign * 100000000 + zone * 1000000 + num * 10 + suffix;
}

string dm_to_string ( int64_t dm )
{
	int64_t sign = dm / 100000000;
	int64_t	zone = ( dm - sign * 100000000 ) / 1000000;
    int64_t num = ( dm - sign * 100000000 - zone * 1000000 ) / 10;
	int64_t suffix = dm - sign * 100000000 - zone * 1000000 - num * 10;
    
    if ( sign )
    	sign = '+';
    else
    	sign = '-';

    if ( suffix == 1 )
    	suffix = 'a';
    else if ( suffix == 2 )
    	suffix = 'b';
    else if ( suffix == 3 )
    	suffix = 'n';
    else if ( suffix == 4 )
    	suffix = 'p';
    else if ( suffix == 5 )
    	suffix = 's';

	if ( suffix > 0 )
	    return format ( "%c%02d %d%c", sign, zone, num, suffix );
	else
    	return format ( "%c%02d %d", sign, zone, num );
}

int64_t string_to_wds ( string str )
{
	char	sign = 0;
	int		ra = 0, dec = 0;
	
	sscanf ( str.c_str(), "%d%c%d", &ra, &sign, &dec );
	
	if ( sign == '+' )
		sign = 1;
	else
		sign = 0;
		
	if ( ra >= 0 && ra < 24000 && dec >= 0 && dec < 9000 )
		return ra * 100000 + sign * 10000 + dec;
	else
		return 0;
}

string wds_to_string ( int64_t wds )
{
	int64_t ra = wds / 100000;
	int64_t sign = ( wds - ra * 100000 ) / 10000;
	int64_t dec = wds - ra * 100000 - sign * 10000;
	
	if ( sign )
		sign = '+';
	else
		sign = '-';

	return format ( "%05d%c%04d", ra, sign, dec );
}

int64_t string_to_ngcic ( string str )
{
	int		num = 0;
	char	ext = 0;

	sscanf ( str.c_str(), "%d%c", &num, &ext );
	
	if ( ext >= 'A' && ext <= 'I' )
		ext = ext - 'A' + 1;
	else if ( ext >= 'a' && ext <= 'i' )
		ext = ext - 'a' + 1;
	else
		ext = 0;
			
    if ( num >= 0 && num <= 7840 )
    	return num * 10 + ext;
    else
    	return 0;
}

string ngcic_to_string ( int64_t ngcic )
{
	int64_t		num = ngcic / 10;
	int64_t		ext = ngcic - num * 10;
	
	if ( ext > 0 )
		return format ( "%d%c", num, ext + 'A' - 1 );
	else
		return format ( "%d", num );
}

static void mapinit ( void )
{
	for ( int i = 0; i < _bayvec.size(); i++ )
		_baymap.insert ( { _bayvec[i], i + 1 } );

	for ( int i = 0; i < _convec.size(); i++ )
		_conmap.insert ( { _convec[i], i + 1 } );
}

SSIdentifier::SSIdentifier ( void )
{
	_id = 0;
}

SSIdentifier::SSIdentifier ( SSCatalog catalog, int64_t ident )
{
	_id = catalog * 10000000000000000LL + ident;
}

SSCatalog SSIdentifier::catalog ( void )
{
	return static_cast<SSCatalog> ( _id / 10000000000000000LL );
}

int64_t SSIdentifier::identifier ( void )
{
	return _id % 10000000000000000LL;
}

SSIdentifier SSIdentifier::fromString ( string str )
{
	size_t len = str.length();
	
	if ( _conmap.size() == 0 || _baymap.size() == 0 )
		mapinit();

    // if string is a number inside paratheses, attempt to parse as an asteroid number
    
    if ( str[0] == '(' && str[len - 1] == ')' )
    {
        int64_t n = strtoint ( str.substr ( 1, len - 2 ) );
        if ( n > 0 )
            return SSIdentifier ( kCatAstNum, n );
    }
    
    // if string is a number followed by "P", parse as a periodic comet number
    
    if ( str.find ( "P" ) != string::npos )
    {
        int64_t n = strtoint ( str.substr ( 0, str.find ( "P" ) ) );
        if ( n > 0 )
            return SSIdentifier ( kCatComNum, n );
    }

    // if string begins with "M", attempt to parse a Messier number
	
	if ( str.find ( "M" ) == 0 && len > 1 )
	{
		int64_t m = strtoint ( str.substr ( 1, len - 1 ) );
		if ( m > 0 && m <= 110 )
			return SSIdentifier ( kCatMessier, m );
	}

	// if string begins with "C", attempt to parse a Caldwell number
	
	if ( str.find ( "C" ) == 0 && len > 1 )
	{
		int64_t c = strtoint ( str.substr ( 1, len - 1 ) );
		if ( c > 0 && c <= 109 )
			return SSIdentifier ( kCatCaldwell, c );
	}

	// if string begins with "NGC", attempt to parse a New General Catalog identifier
	
	if ( str.find ( "NGC" ) == 0 && len > 3 )
	{
		int64_t ngc = string_to_ngcic ( str.substr ( 3, len - 3 ) );
		if ( ngc )
			return SSIdentifier ( kCatNGC, ngc );
	}

	// if string begins with "ICC", attempt to parse an Index Catalog identifier
	
	if ( str.find ( "IC" ) == 0 && len > 2 )
	{
		int64_t ic = string_to_ngcic ( str.substr ( 2, len - 2 ) );
		if ( ic )
			return SSIdentifier ( kCatIC, ic );
	}

	// if string begins with "HR", attempt to parse a Harvard Revised (Bright Star) catalog identifier
	
	if ( str.find ( "HR" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatHR, stoi ( str.substr ( pos, len - pos ) ) );
	}
	
	// if string begins with "HD", attempt to parse a Henry Draper catalog identifier
	
	if ( str.find ( "HD" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatHD, stoi ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "SAO", attempt to parse a Smithsonian Astrophyiscal Observatory catalog identifier
	
	if ( str.find ( "SAO" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatSAO, stoi ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "HIP", attempt to parse a Hipparcos catalog identifier
	
	if ( str.find ( "HIP" ) == 0 )
	{
		size_t pos = str.find_first_of ( "0123456789" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatHIP, stoi ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "BD" or "SD", attempt to parse a Bonner Durchmusterung catalog identifier
	// Note: "SD" is abbrevieation for Southern Durchmusterung, found in SKY2000 Master Star Catalog.
	
	if ( str.find ( "BD" ) == 0 || str.find ( "SD" ) == 0 )
	{
		size_t pos = str.find_first_of ( "+-" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatBD, string_to_dm ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "CD", attempt to parse a Bonner Durchmusterung catalog identifier
	
	if ( str.find ( "CD" ) == 0 )
	{
		size_t pos = str.find_first_of ( "+-" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatCD, string_to_dm ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "CP", attempt to parse a Bonner Durchmusterung catalog identifier
	
	if ( str.find ( "CP" ) == 0 )
	{
		size_t pos = str.find_first_of ( "+-" );
		if ( pos != string::npos )
			return SSIdentifier ( kCatCP, string_to_dm ( str.substr ( pos, len - pos ) ) );
	}

	// if string begins with "WDS", attempt to parse a Washington Double Star catalog identifier
	
	if ( str.find ( "WDS" ) == 0 && len > 3 )
	{
		int64_t wds = string_to_wds ( str.substr ( 3, len - 3 ) );
		if ( wds )
			return SSIdentifier ( kCatWDS, wds );
	}
	
	// parse constellation abbreviation from last 3 characters of string
	
	string constr = "";
	size_t consep = str.find_first_of ( " " );
	if ( len >= 3 )
		constr = str.substr ( len - 3, 3 );
	
	// get constellation number; return unknown id if abbreviation not recognized
	
	int con = _conmap[ constr ];
	if ( ! con )
		return SSIdentifier ( kCatUnknown, 0 );
	
	// try parsing prefix as a variable star designation; return GCVS identifier if successful.
	
	string varstr = str.substr ( 0, consep );
	int64_t var = string_to_gcvs ( varstr );
	if ( var > 0 )
		return SSIdentifier ( kCatGCVS, var * 100 + con );
	
	// Find numeric portion of string, if any, and convert to integer
	
	string numstr = "";
	size_t numsep = str.find_first_of ( "0123456789" );
	if ( numsep != string::npos )
		numstr = str.substr ( numsep, str.find_last_of ( "0123456789" ) - numsep + 1 );
	int num = numstr.empty() ? 0 : stoi ( numstr );
	
	// If string begins with a number, return a Flamsteed catalog identification
	
	if ( numsep == 0 )
		return SSIdentifier ( kCatFlamsteed, num * 100 + con );

	// Otherwise, extract first non-numeric part of string and parse as a Bayer designation
	// with the numeric portion (if any) as superscript
	
	string baystr = "";
	if ( numsep == string::npos )
		baystr = str.substr ( 0, consep );
	else
		baystr = str.substr ( 0, numsep );
	
	int bay = string_to_bayer ( baystr );
	if ( bay > 0 )
		return SSIdentifier ( kCatBayer, ( bay * 100 + num ) * 100 + con );
	
	// Return unknown identifier
	
	return SSIdentifier ( kCatUnknown, 0 );
}

string SSIdentifier::toString ( void )
{
	if ( _conmap.size() == 0 || _baymap.size() == 0 )
		mapinit();

	SSCatalog cat = catalog();
	int64_t id = identifier();
	string str = "";
	
	if ( cat == kCatBayer )
	{
		int64_t bay = id / 10000;
		int64_t num = ( id - bay * 10000 ) / 100;
		int64_t con = id % 100;
		
		string baystr = bayer_to_string ( bay );
		string constr = _convec[con - 1];
		if ( num > 0 )
			str = baystr + to_string ( num ) + " " + constr;
		else
			str = baystr + " " + constr;
	}
	else if ( cat == kCatFlamsteed )
	{
		int64_t num = id / 100;
		int64_t con = id % 100;
		str = to_string ( num ) + " " + _convec[con - 1];
	}
	else if ( cat == kCatGCVS )
	{
		int64_t num = id / 100;
		int64_t con = id % 100;
		str = gcvs_to_string ( num ) + " " + _convec[con - 1];
	}
	else if ( cat == kCatHR )
	{
		str = "HR " + to_string ( id );
	}
	else if ( cat == kCatHD )
	{
		str = "HD " + to_string ( id );
	}
	else if ( cat == kCatSAO )
	{
		str = "SAO " + to_string ( id );
	}
	else if ( cat == kCatHIP )
	{
		str = "HIP " + to_string ( id );
	}
	else if ( cat == kCatBD )
	{
		str = "BD " + dm_to_string ( id );
	}
	else if ( cat == kCatCD )
	{
		str = "CD " + dm_to_string ( id );
	}
	else if ( cat == kCatCP )
	{
		str = "CP " + dm_to_string ( id );
	}
	else if ( cat == kCatWDS )
	{
		str = "WDS " + wds_to_string ( id );
	}
	else if ( cat == kCatMessier )
	{
		str = "M " + to_string ( id );
	}
	else if ( cat == kCatCaldwell )
	{
		str = "C " + to_string ( id );
	}
	else if ( cat == kCatNGC )
	{
		str = "NGC " + ngcic_to_string ( id );
	}
	else if ( cat == kCatIC )
	{
		str = "IC " + ngcic_to_string ( id );
	}
    else if ( cat == kCatAstNum )
    {
        str = "(" + to_string ( id ) + ")";
    }
    else if ( cat == kCatComNum )
    {
        str = to_string ( id ) + "P";
    }
    else if ( cat == kCatNORADSat )
    {
        str = to_string (id);
    }
    
	return str;
}

bool compareSSIdentifiers ( const SSIdentifier &id1, const SSIdentifier &id2 )
{
	return id1 < id2;
}

// Adds a new identifier to a vector of identifiers,
// if the new identifier is valid and not already present in the vector.

void addIdentifier ( vector<SSIdentifier> &identVec, SSIdentifier ident )
{
    if ( ident && find ( identVec.begin(), identVec.end(), ident ) == identVec.end() )
        identVec.push_back ( ident );
}