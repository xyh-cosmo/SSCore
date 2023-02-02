// SSImportGAIADR3.cpp
// SSCore
//
// Created by Tim DeBenedictis on 1/29/23.
// Copyright ©2023 Southern Stars Group, LLC. All rights reserved.

#include <algorithm>
#include <iostream>
#include <fstream>
#include <dirent.h>

#include "SSImportGAIADR3.hpp"
#include "SSUtilities.hpp"

#define GAIADR3_HIP2_NUM_FIELDS 5
#define GAIADR3_TYC2_NUM_FIELDS 6
#define GAIADR3_SOURCE_NUM_FIELDS 152

struct SSGAIADir
{
    string  root;               // path to root directory containing gzipped GAIA source data files in CSV format
    DIR     *dp = nullptr;      // pointer to directory
    gzFile  fp = nullptr;       // pointer to gzipped data file
};

// Opens the GAIA source data file directory with the given path (root)
// Returns pointer to GAIADir structure if successful or nullptr on failure.

SSGAIADir *OpenGAIADir ( const string &root )
{
    SSGAIADir *gdp = new SSGAIADir();
    if ( gdp == NULL )
        return nullptr;
    
    gdp->dp = opendir ( root.c_str() );
    if ( gdp->dp == nullptr )
    {
        delete gdp;
        return nullptr;
    }
    
    gdp->root = root;
    return gdp;
}

// Closes GAIADir (gdp) and releases memory; gdp pointer will be invalid after return.

void SSCloseGAIADir ( SSGAIADir *gdp )
{
    closedir ( gdp->dp );
    gzclose ( gdp->fp );
    delete gdp;
}

// Reads one GAIA DR3 source record from directory (gdp) into (rec)
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file.

int SSReadGAIADir ( SSGAIADir *gdp, SSGAIADR3SourceRecord &rec )
{
    int     result = 0;
    string  csv_gz_path;

    if ( gdp == NULL || gdp->dp == NULL )
        return EOF;
    
    if ( gdp->fp )
    {
        result = SSReadGAIADR3SourceRecord ( gdp, rec );
        if ( result != EOF )
            return result;

        gzclose ( gdp->fp );
        gdp->fp = NULL;
    }

    while ( gdp->fp == NULL )
    {
        struct dirent *pDirEnt = readdir ( gdp->dp );
        if ( pDirEnt == NULL )
            return EOF;
        
        size_t len = strlen ( pDirEnt->d_name );
        if ( len < 6 || strcmp ( pDirEnt->d_name + len - 6, "csv.gz" ) != 0 )
            continue;
        
        csv_gz_path = gdp->root;
        if ( csv_gz_path.back() != '/' )
            csv_gz_path += '/';
        csv_gz_path += pDirEnt->d_name;
        
        gdp->fp = gzopen ( csv_gz_path.c_str(), "rb" );
    }

    return SSReadGAIADir ( gdp, rec );
}

// Reads one CSV record from gzip-compressed GAIA data file.
// Returns number of fields in record or -1 (EOF) on failure.
// Field contents are returned in vector of strings (fields)

static int SSReadGAIACSVRecord ( gzFile gz_fp, vector<string> &fields )
{
    int i = 0;
    char csv_buf[4086] = { 0 };
    
    if ( gzgets ( gz_fp, csv_buf, sizeof ( csv_buf ) ) == NULL )
        return ( EOF );
    
    for ( i = (int) strlen ( csv_buf ) - 1; i > 0; i-- )
        if ( csv_buf[i] == '\r' || csv_buf[i] == '\n' )
            csv_buf[i] = 0;
        else
            break;
    
    fields = split_csv ( csv_buf );
    return (int) fields.size();
}

// Reads one record from gzip-compressed GAIA DR3 source file.
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file

int SSReadGAIADR3SourceRecord ( SSGAIADir *gdp, SSGAIADR3SourceRecord *pRec )
{
    vector<string> fields;
    int result = SSReadGAIACSVRecord ( gdp->fp, fields );
    if ( result == EOF )
        return result;
    if ( result < GAIADR3_SOURCE_NUM_FIELDS )
        return false;
    
    pRec->solution_id = strtoint64 ( fields[0] );
    pRec->source_id = strtoint64 ( fields[2] );
    pRec->ref_epoch = strtoint64 ( fields[4] );
    pRec->ra = strtofloat64 ( fields[5] );
    pRec->ra_error = strtofloat64 ( fields[6] );
    pRec->dec = strtofloat64 ( fields[7] );
    pRec->dec_error = strtofloat64 ( fields[8] );
    pRec->parallax = strtofloat64 ( fields[9] );
    pRec->parallax_error = strtofloat64 ( fields[10] );
    pRec->pmra = strtofloat64 ( fields[13] );
    pRec->pmra_error = strtofloat64 ( fields[14] );
    pRec->pmdec = strtofloat64 ( fields[15] );
    pRec->pmdec_error = strtofloat64 ( fields[16] );
    pRec->duplicated_source = fields[64][0] == 'T' ? true : false;
    pRec->phot_g_mean_mag = strtofloat ( fields[69] );
    pRec->phot_bp_mean_mag = strtofloat ( fields[74] );
    pRec->phot_rp_mean_mag = strtofloat ( fields[79] );
    pRec->radial_velocity = strtofloat ( fields[89] );
    pRec->radial_velocity_error = strtofloat ( fields[90] );
    pRec->vbroad = strtofloat ( fields[104] );
    pRec->vbroad_error = strtofloat ( fields[105] );
    pRec->phot_variable_flag = fields[111][0];
    pRec->teff_gspphot = strtofloat ( fields[130] );
    pRec->logg_gspphot = strtofloat ( fields[133] );
    pRec->mh_gspphot = strtofloat ( fields[136] );
    pRec->distance_gspphot = strtofloat ( fields[139] );
    pRec->azero_gspphot = strtofloat ( fields[142] );
    pRec->ag_gspphot = strtofloat ( fields[145] );
    pRec->ebpminrp_gspphot = strtofloat ( fields[148] );
    
    // record must have a non-zero solution and source ID to be valid
    
    return ( pRec->solution_id && pRec->source_id ) ? true : false;
}

// Reads one record from a GAIA DR3 <-> Hipparcos 2 cross-match file.
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file

static int SSReadGAIADR3Hipparcos2CrossMatchRecord ( gzFile gz_fp, SSGAIACrossMatchRecord &record )
{
    vector<string> fields;
    int result = SSReadGAIACSVRecord ( gz_fp, fields );
    if ( result == EOF )
        return result;
    if ( result < GAIADR3_HIP2_NUM_FIELDS )
        return false;

    record.source_id = strtoint64 ( fields[0] );
    record.ext_source_id = strtoint ( fields[1] );
    record.angular_distance = strtofloat ( fields[2] );
    record.number_of_neighbours = strtoint ( fields[3] );
    record.xm_flag = strtoint ( fields[4] );

    return ( record.source_id && record.ext_source_id ) ? true : false;
}

// Reads one record from a GAIA DR3 <-> Tycho 2 cross-match file.
// Returns 1 (true) if successful, 0 (false) on parse failure, or -1 (EOF) on end-of-file

static int SSReadGAIADR3Tycho2CrossMatchRecord ( gzFile gz_fp, SSGAIACrossMatchRecord &record )
{
    vector<string> fields;
    int result = SSReadGAIACSVRecord ( gz_fp, fields );
    if ( result == EOF )
        return result;
    
    record.source_id = strtoint64 ( fields[0] );
    
    int rgn = 0, num = 0, ext = 0;
    if ( sscanf ( fields[1].c_str(), "%d-%d-%d", &rgn, &num, &ext ) == 3 )
        record.ext_source_id = ext + num * 10 + rgn * 1000000;
    else
        record.ext_source_id = 0;
    
    record.angular_distance = strtofloat ( fields[2] );
    record.xm_flag = strtoint ( fields[3] );
    record.number_of_neighbours = strtoint ( fields[5] );
    
    return ( record.source_id && record.ext_source_id ) ? true : false;
}

// Reads a GAIA cross-match file from the specified path.
// Returns number of records read from file.
// Records are returned in the map of SSGAIACrossMatchRecords (records)

int SSReadGAIACrossMatchFile ( const string &path, SSGAIACrossMatchFile cmf, SSGAIACrossMatch &records )
{
    int       result = 0;
    gzFile    gzfp = NULL;

    gzfp = gzopen ( path.c_str(), "r" );
    if ( gzfp == NULL )
        return NULL;
    
    while ( true )
    {
        SSGAIACrossMatchRecord    record = { 0 };
        
        if ( cmf == kHipparcos2CrossMatch )
            result = SSReadGAIADR3Hipparcos2CrossMatchRecord ( gzfp, record );
        else if ( cmf == kTycho2CrossMatch )
            result = SSReadGAIADR3Tycho2CrossMatchRecord ( gzfp, record );

        if ( result == EOF )
            break;
        
        if ( result == true )
            records[ record.source_id ] = record;
    }

    gzclose ( gzfp );
    return (int) records.size();
}

float GAIADR2JohnsonV ( float g, float gbp, float grp )
{
    float gbp_grp = gbp && grp ? gbp - grp : 0.0;
    float gbp_grp2 = gbp_grp * gbp_grp;
    float g_v = -0.01760 - 0.00686 * gbp_grp - 0.1732 * gbp_grp2;
    float v = g - g_v;
    
    return v;
}

float GAIADR2JohnsonI ( float g, float gbp, float grp )
{
    float gbp_grp = gbp && grp ? gbp - grp : 0.0;
    float gbp_grp2 = gbp_grp * gbp_grp;
    float g_i = 0.02085 + 0.7419 * gbp_grp - 0.09631 * gbp_grp2;
    float i = g - g_i;
    
    return i;
}

float GAIADR2JohnsonB ( float g, float gbp, float grp )
{
    float v = GAIADR2JohnsonV ( g, gbp, grp );
    float i = GAIADR2JohnsonI ( g, gbp, grp );
    float v_i = v && i ? v - i : 0.0;
    float v_i2 = v_i * v_i;
    float v_i3 = v_i * v_i2;
    float g_b = 0.1201 - 1.2668 * v_i - 0.0044 * v_i2 + 0.0020 * v_i3;
    float b = g - g_b;
    
    return b;
}

// Exports GAIA DR3 "essentials" from full GAIA source catalog.
// Gzipped GAIA DR2 source files are stored in the root directory.
// Essentials file is written to the output file at (outpath).
// Hipparcos (hipCM) and Tycho (tycCM) cross-match indexes should have been read previously.
// GAIA sources brighter than gmin or fainter than gmax will be discarded.

int SSExportGAIADR3StarData ( const string &root, const string &outpath, const SSGAIACrossMatch &hipCM, const SSGAIACrossMatch &tycCM, float gmin, float gmax )
{
    int     i, n_records = 0, result = 0;
    FILE    *outfile = NULL;
    double  startJD = SSTime::fromSystem().jd, endJD = 0;

    // Open GAIA root directory
    
    SSGAIADir *gdp = OpenGAIADir ( root );
    if ( gdp == NULL )
    {
        printf ( "Can't open GAIA directory!\n" );
        return -1;
    }
    
    printf ( "Opened GAIA directory.\n" );
    
    // Attempt to open output file. Return error code on failure.
    
    outfile = fopen ( outpath.c_str(), "wb" );
    if ( outfile == NULL )
    {
        printf ( "Can't open output file %s!\n", outpath.c_str() );
        SSCloseGAIADir ( gdp );
        return -1;
    }
    
    // Read records from GAIA directory individually, until we hit end-of-file.
    // Append "essentials" records to the array.
    
    vector<SSGAIARec> array;
    while ( true )
    {
        SSGAIADR3SourceRecord record = { 0 };
        result = SSReadGAIADir ( gdp, &record );
        if ( result == EOF )
            break;
        
        // skip over invalid records
        
        if ( result == false )
            continue;
        
        n_records++;
        if ( n_records % 1000000 == 0 )
            printf ( "Read %d GAIA records...\n", n_records );

        // Strip out stars brighter than minimum, or fainter than maximum, G magnitude
        
        if ( record.phot_g_mean_mag < gmin || record.phot_g_mean_mag > gmax )
            continue;
        
        SSGAIARec outrec = { 0 };
        outrec.source_id = record.source_id;
        
        auto it = hipCM.find ( record.source_id );
        if ( it != hipCM.end() )
            outrec.hip_source_id = it->second.ext_source_id;

        it = tycCM.find ( record.source_id );
        if ( it != tycCM.end() )
            outrec.tyc_source_id = it->second.ext_source_id;
        
        outrec.ra_mas = record.ra * 3600000.0;
        outrec.dec_mas = record.dec * 3600000.0;
        outrec.pos_error = sqrt ( record.ra_error * record.ra_error + record.dec_error * record.dec_error );
        outrec.parallax = record.parallax;
        outrec.parallax_error = record.parallax_error;
        outrec.pmra_mas = record.pmra;
        outrec.pmdec_mas = record.pmdec;
        outrec.pm_error = sqrt ( record.pmra_error * record.pmra_error + record.pmdec_error * record.pmdec_error );
        outrec.phot_g_mean_mmag = record.phot_g_mean_mag * 1000.0;
        outrec.phot_bp_mean_mmag = record.phot_bp_mean_mag * 1000.0;
        outrec.phot_rp_mean_mmag = record.phot_rp_mean_mag * 1000.0;
        outrec.radial_velocity = record.radial_velocity;
        outrec.radial_velocity_error = record.radial_velocity_error;
        
        array.push_back ( outrec );
    }
    
    // Write array of output GAIA "essentials" records to file.
    
    n_records = (int) array.size();
    for ( i = 0; i < n_records; i++ )
        if ( fwrite ( &array[i], sizeof ( array[i] ), 1, outfile ) != 1 )
            break;

    // Free memory for array anc close file.
    
    fclose ( outfile );
    printf ( "Wrote %d records to %s, file closed.\n", i, outpath.c_str() );
    
    endJD = SSTime::fromSystem().jd;
    printf ( "Elapsed Time: %.02f min\n", SSTime::kMinutesPerDay * ( endJD - startJD ) );
    
    return i;
}

int SSImportGAIA17 ( const string &filename, SSObjectArray &stars )
{
    // Open file; return on failure.

    ifstream file ( filename, ios::binary );
    if ( ! file )
        return 0;

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int numStars = 0;
    
    SSGAIARec gaia = { 0 };
    
    while ( true )
    {
        file.read ( (char *) &gaia, sizeof ( gaia ) );
        if ( ! file )
            break;
        
        if ( gaia.hip_source_id != 0 || gaia.tyc_source_id == 0 )
            continue;
        
        SSSpherical coords ( SSAngle::fromArcsec ( gaia.ra_mas / 1000.0 ), SSAngle::fromArcsec ( gaia.dec_mas / 1000.0 ), INFINITY );
        if ( gaia.parallax > 0.0 && gaia.parallax < 100.0 )
            coords.rad = 1000.0 * SSCoordinates::kLYPerParsec / gaia.parallax;
        
        if ( ::isinf ( coords.rad ) )
            continue;
        
        SSSpherical motion ( SSAngle::fromArcsec ( gaia.pmra_mas / 1000.0 ) / cos ( coords.lat ), SSAngle::fromArcsec ( gaia.pmdec_mas / 1000.0 ), INFINITY );
        if ( gaia.radial_velocity != 0 && gaia.radial_velocity_error != 0 )
            motion.rad = gaia.radial_velocity / SSCoordinates::kLightKmPerSec;
                      
        // Apply proper motion from epoch 2015.5 to epoch 2000
        
        coords.lon -= motion.lon * 15.5;
        coords.lat -= motion.lat * 15.5;

        vector<SSIdentifier> idents;
        if ( gaia.hip_source_id )
            idents.push_back ( SSIdentifier ( kCatHIP, gaia.hip_source_id ) );
        
        if ( gaia.tyc_source_id )
            idents.push_back ( SSIdentifier ( kCatTYC, gaia.tyc_source_id ) );

        if ( gaia.source_id )
            idents.push_back ( SSIdentifier ( kCatGAIA, gaia.source_id ) );

        float vmag = GAIADR2JohnsonV ( gaia.phot_g_mean_mmag / 1000.0, gaia.phot_bp_mean_mmag / 1000.0, gaia.phot_rp_mean_mmag / 1000.0 );
        float bmag = GAIADR2JohnsonB ( gaia.phot_g_mean_mmag / 1000.0, gaia.phot_bp_mean_mmag / 1000.0, gaia.phot_rp_mean_mmag / 1000.0 );

        // Construct star and insert into star vector.

        SSObjectPtr pObj = SSNewObject ( kTypeStar );
        SSStarPtr pStar = SSGetStarPtr ( pObj );
        if ( pStar != nullptr )
        {
            pStar->setIdentifiers ( idents );
            pStar->setFundamentalMotion ( coords, motion );
            pStar->setVMagnitude ( vmag );
            pStar->setBMagnitude ( bmag );
            
            // cout << pStar->toCSV() << endl;
            stars.append ( pObj );
            numStars++;
        }
    }
    
    return numStars;
}
