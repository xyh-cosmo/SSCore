//  SSStar.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/15/20.
//  Copyright © 2020 Southern Stars. All rights reserved.

#include <algorithm>
#include <map>

#include "SSCoordinates.hpp"
#include "SSStar.hpp"

// Constructs single star with a specific object type code.
// All fields except type code are set to empty strings or infinity,
// signifying unknown/undefined values.

SSStar::SSStar ( SSObjectType type ) : SSObject ( type )
{
    _names = vector<string> ( 0 );
    _idents = vector<SSIdentifier> ( 0 );

    _parallax = 0.0;
    _radvel = INFINITY;
    _position = _velocity = SSVector ( INFINITY, INFINITY, INFINITY );
    _Vmag = INFINITY;
    _Bmag = INFINITY;
    
    _spectrum = "";
}

// Constructs single star with type code set to indicate "single star".
// All other fields set to values, signifying unknown/undefined.

SSStar::SSStar ( void ) : SSStar ( kTypeStar )
{

}

// Constructs variable star with all fields except type code
// set to empty strings or infinity, signifying "unknown".

SSVariableStar::SSVariableStar ( void ) : SSStar ( kTypeVariableStar )
{
    _varType = "";
    _varMaxMag = INFINITY;
    _varMinMag = INFINITY;
    _varPeriod = INFINITY;
    _varEpoch = INFINITY;
}

// Constructs double star with all fields except type code
// set to empty strings or infinity, signifying "unknown".

SSDoubleStar::SSDoubleStar ( void ) : SSStar ( kTypeDoubleStar )
{
    _comps = "";
    _magDelta = INFINITY;
    _sep = INFINITY;
    _PA = INFINITY;
    _PAyr = INFINITY;
}

// Constructs double variable star with all fields except type code
// set to empty strings or infinity, signifying "unknown".

SSDoubleVariableStar::SSDoubleVariableStar ( void ) : SSDoubleStar(), SSVariableStar()
{
    _type = kTypeDoubleVariableStar;
}

// Constructs deep sky object with the specified type code;
// all other fields are set to unknown/undefined values.

SSDeepSky::SSDeepSky ( SSObjectType type ) : SSStar ( type )
{
    _majAxis = INFINITY;
    _minAxis = INFINITY;
    _PA = INFINITY;
}

// Returns i-th identifier in this star's ident vector,
// or null identifier (i.e. zero) if i is out of range

SSIdentifier SSStar::getIdentifier ( int i )
{
    if ( i >= 0 && i < _idents.size() )
        return _idents[i];
    else
        return SSIdentifier();
}

// Returns this star's identifier in a specific catalog.
// If not present, returns null identifier (i.e. zero).

SSIdentifier SSStar::getIdentifier ( SSCatalog cat )
{
    for ( int i = 0; i < _idents.size(); i++ )
        if ( _idents[i].catalog() == cat )
            return _idents[i];
    
    return SSIdentifier();
}

bool SSStar::addIdentifier ( SSIdentifier ident )
{
    return SSAddIdentifier ( ident, _idents );
}

void SSStar::sortIdentifiers ( void )
{
    sort ( _idents.begin(), _idents.end(), compareSSIdentifiers );
}

// Compute star's heliocentric position and velocity in AU and AU per day in the fundamental (J2000 mean equatorial)
// reference frame at the Julian Ephemeris Date specified inside the SSCoordinates object.
// If star's parallax is unknown, returned position will be approximately a unit vector.
// If parallax is known, position vector magnitude will be > 206265 (i.e., 1 parsec in AU).

void SSStar::computePositionVelocity ( SSCoordinates &coords, SSVector &pos, SSVector &vel )
{
    // Start by assuming star's heliocentric position is unchanged from J2000.
    
    pos = _position;
    vel = _velocity;
    
    // If applying stellar space motion, and the star's space motion is known, add its space velocity
    // (times years since J2000) to its J2000 position.

    if ( coords.getStarMotion() && ! isinf ( _velocity.x ) )
        pos += _velocity * ( coords.getJED() - SSTime::kJ2000 ) / SSTime::kDaysPerJulianYear;
    
    // If star's parallax is known, scale position and velocity by parallax to AU and AU/day.
    
    if ( _parallax > 0.0 )
    {
        pos *= coords.kAUPerParsec / _parallax;
        vel *= SSTime::kDaysPerJulianYear * coords.kAUPerParsec / _parallax;
    }
}

// Compute star's apparent direction, distance, and magnitude at the Julian Ephemeris Date
// specified inside the SSCoordinates object.

void SSStar::computeEphemeris ( SSCoordinates &coords )
{
    // Start by assuming star's current apparent direction vector is unchanged from J2000.
    
    _direction = _position;

    // If applying stellar space motion, and the star's space motion is known, add its space velocity
    // (times years since J2000) to its J2000 position.

    if ( coords.getStarMotion() && ! isinf ( _velocity.x ) )
        _direction += _velocity * ( coords.getJED() - SSTime::kJ2000 ) / SSTime::kDaysPerJulianYear;
    
    // If applying heliocentric parallax, and the star's parallax is known, subtract the observer's
    // position divided by the star's J2000 distance.
    
    if ( coords.getStarParallax() && _parallax > 0.0 )
        _direction -= coords.getObserverPosition() * ( _parallax / coords.kAUPerParsec );

    // If star's apparent direction is the same as in J2000, we ignored both its space motion and parallax.
    // If star's parallax is known, convert to distance in AU; otherwise set distance to infinity.
    // Star's current visual magnitude equals its J2000 magnitude.

    if ( _direction == _position )
    {
        _distance = _parallax > 0.0 ? coords.kAUPerParsec / _parallax : INFINITY;
        _magnitude = _Vmag < INFINITY ? _Vmag : _Bmag;
    }
    else
    {
        // If we applied stellar space motion or parallax, compute "delta" (ratio of star's current distance
        // to its J2000 distance). Then normalize direction to unit vector. If star's J2000 parallax is known,
        // get its current distance in AU. Get current visual magnitude by adjusting J2000 magnitude for delta.

        double delta = _direction.magnitude();
        _direction = _direction / delta;
        _distance = _parallax > 0.0 ? delta * coords.kAUPerParsec / _parallax : INFINITY;
        _magnitude = ( _Vmag < INFINITY ? _Vmag : _Bmag ) + 5.0 * log10 ( delta );
    }

    // Finally apply aberration of light, if desired.
    
    if ( coords.getAberration() )
        _direction = coords.applyAberration ( _direction );
}

// Sets this star's spherical coordinates and distance in the fundamental frame,
// i.e. the star's mean equatorial J2000 coordinates at epoch 2000.
// The star's RA (coords.lon) and Dec (coords.lat) are in radians.
// The star's distance in light years (coords.rad) may be infinite if unknown.

void SSStar::setFundamentalCoords ( SSSpherical coords )
{
    _parallax = isinf ( coords.rad ) ? 0.0 : SSCoordinates::kLYPerParsec / coords.rad;
    _position = SSVector ( coords.lon, coords.lat, 1.0 );
}

// Sets this star's spherical coordinates and proper motion in the fundamental frame
// i.e. the star's mean equatorial J2000 coordinates and proper motion at epoch 2000.
// The star's RA (coords.lon) and Dec (coords.lat) are in radians.
// The stars proper motion in RA (motion.ra) and dec (motion.dec) are in radians per Julian year.
// The star's distance in light years (coords.rad) may be infinite if unknown.
// The star's radial velocity in light years per year (motion.rad) may be infinite if unknown.
// Mathematically, both coordinates and motion are required to compute the star's rectangular
// heliocentric position and motion; practically, if you have its motion you'll also have its position,
// so we pass them both here.  You can extract them separately (see below).

void SSStar::setFundamentalMotion ( SSSpherical coords, SSSpherical motion )
{
    _parallax = isinf ( coords.rad ) ? 0.0 : SSCoordinates::kLYPerParsec / coords.rad;
    _radvel = motion.rad;

    // if distance or radial velocity are unknown, treat them as zero;
    // otherwise divide radial velocity by distance; set unit distance.
    
    if ( isinf ( coords.rad ) || isinf ( motion.rad ) )
        motion.rad = 0.0;
    else
        motion.rad /= coords.rad;
    
    coords.rad = 1.0;
    
    _position = coords.toVectorPosition();
    
    if ( isinf ( motion.lon ) || isinf ( motion.lat ) )
        _velocity = SSVector ( INFINITY, INFINITY, INFINITY );
    else
        _velocity = coords.toVectorVelocity ( motion );
}

// Returns this star's heliocentric spherical coordinates in the fundamental
// J2000 mean equatorial frame at epoch J2000.  The star's RA (coords.lon)
// and Dec (coords.lat) are in radians.  Its distance in light years (coords.rad)
// will be infinite if unknown.

SSSpherical SSStar::getFundamentalCoords ( void )
{
    SSSpherical coords = _position.toSpherical();
    coords.rad = ( isinf ( _parallax ) || _parallax == 0.0 ) ? INFINITY : SSCoordinates::kLYPerParsec / _parallax;
    return coords;
}

// Returns this star's heliocentric proper motion in the fundamental J2000
// mean equatorial frame at epoch J2000.  The proper motion in RA (motion.lon)
// and Dec (motion.lat) are both in radians per year.  Its radial velocity
// (motion.rad) is in light years per year and will be infinite if unknown.

SSSpherical SSStar::getFundamentalMotion ( void )
{
    SSSpherical motion = _position.toSphericalVelocity ( _velocity );
    motion.rad = _radvel;
    return motion;
}

// Converts B-V color index (bv) to RGB color.
// B-V will be clamped to range -0.4 to +2.0.
// RGB values will be returned in the range 0.0 to 1.0.
// from https://stackoverflow.com/questions/21977786/star-b-v-color-index-to-apparent-rgb-color

void SSStar::bmv2rgb ( float bv, float &r, float &g, float &b )
{
    double t = r = g = b = 0.0;

    if ( bv < -0.4 )
        bv = -0.4;
    
    if ( bv > 2.0 )
        bv = 2.0;

    // red
    
    if ( bv >= -0.40 && bv < 0.00 )
    {
        t = ( bv + 0.40 ) / ( 0.00 + 0.40 );
        r = 0.61 + ( 0.11 * t ) + ( 0.1 * t * t );
    }
    else if ( bv >= 0.00 && bv < 0.40 )
    {
        t = ( bv - 0.00 ) / ( 0.40 - 0.00 );
        r = 0.83 + ( 0.17 * t );
    }
    else if ( bv >= 0.40 && bv < 2.10 )
    {
        t = ( bv - 0.40 ) / ( 2.10 - 0.40 );
        r = 1.00;
    }
    
    // green
    
    if ( bv >= -0.40 && bv < 0.00 )
    {
        t = ( bv + 0.40 ) / ( 0.00 + 0.40 );
        g = 0.70 + ( 0.07 * t ) + ( 0.1 * t * t );
    }
    else if ( bv >= 0.00 && bv < 0.40 )
    {
        t = ( bv - 0.00 ) / ( 0.40 - 0.00 );
        g = 0.87 + ( 0.11 * t );
    }
    else if ( bv >= 0.40 && bv < 1.60 )
    {
        t = ( bv - 0.40 ) / ( 1.60 - 0.40 );
        g = 0.98 - ( 0.16 * t );
    }
    else if ( bv >= 1.60 && bv < 2.00 )
    {
        t = ( bv - 1.60 ) / ( 2.00 - 1.60 );
        g = 0.82 - ( 0.5 * t * t );
    }
    
    // blue
    
    if ( bv >= -0.40 && bv < 0.40 )
    {
        t = ( bv + 0.40 ) / ( 0.40 + 0.40 );
        b = 1.00;
    }
    else if ( bv >= 0.40 && bv < 1.50 )
    {
        t = ( bv - 0.40 ) / ( 1.50 - 0.40 );
        b = 1.00 - ( 0.47 * t ) + ( 0.1 * t * t );
    }
    else if ( bv >= 1.50 && bv < 1.94 )
    {
        t = ( bv - 1.50 ) / ( 1.94 - 1.50 );
        b = 0.63 - ( 0.6 * t * t );
    }
}

// Converts B-V color index (bv) to temperature in Kelvin.
// from https://en.wikipedia.org/wiki/Color_index
// Superseded by colorTemperature(), below.

float SSStar::bmv2temp ( float bv )
{
   return 4600.0 * ( ( 1.0 / ( ( 0.92 * bv ) + 1.7 ) ) + ( 1.0 / ( ( 0.92 * bv ) + 0.62 ) ) );
}

// Converts B-V index to stellar surface effective temperature in Kelvins.
// From Table 2 of "ON THE USE OF EMPIRICAL BOLOMETRIC CORRECTIONS FOR STARS",
// Guillermo Torres, Harvard-Smithsonian Center for Astrophysics, 2010.
// https://iopscience.iop.org/article/10.1088/0004-6256/140/5/1158/pdf

float SSStar::colorTemperature ( float bv, int lumclass )
{
    float t = 0.0;
    
    if ( lumclass <= LumClass::Ib )
    {
        t = 4.012559732366214
          - 1.055043117465989 * bv
          + 2.133394538571825 * bv * bv
          - 2.459769794654992 * bv * bv * bv
          + 1.349423943497744 * bv * bv * bv * bv
          - 0.283942579112032 * bv * bv * bv * bv * bv;
    }
    else
    {
        t = 3.979145106714099
          - 0.654992268598245 * bv
          + 1.740690042385095 * bv * bv
          - 4.608815154057166 * bv * bv * bv
          + 6.792599779944473 * bv * bv * bv * bv
          - 5.396909891322525 * bv * bv * bv * bv * bv
          + 2.192970376522490 * bv * bv * bv * bv * bv * bv
          - 0.359495739295671 * bv * bv * bv * bv * bv * bv * bv;
    }
    
    return pow ( 10.0, t );
}

// Converts B-V index to bolometric correction in magnitudes.
// From Table 1 of "ON THE USE OF EMPIRICAL BOLOMETRIC CORRECTIONS FOR STARS",
// Guillermo Torres, Harvard-Smithsonian Center for Astrophysics, 2010.
// https://iopscience.iop.org/article/10.1088/0004-6256/140/5/1158/pdf

float SSStar::bolometricCorrection ( float t )
{
    float bc = INFINITY;
    
    t = log10 ( t );
    if ( t > 3.9 )
    {
        bc = -0.118115450538963E+06
           + 0.137145973583929E+06 * t
           - 0.636233812100225E+05 * t * t
           + 0.147412923562646E+05 * t * t * t
           - 0.170587278406872E+04 * t * t * t * t
           + 0.788731721804990E+02 * t * t * t * t * t;
    }
    else if ( t > 3.7 )
    {
        bc = -0.370510203809015E+05
           + 0.385672629965804E+05 * t
           - 0.150651486316025E+05 * t * t
           + 0.261724637119416E+04 * t * t * t
           - 0.170623810323864E+03 * t * t * t * t;
    }
    else
    {
        bc = -0.190537291496456E+05
           + 0.155144866764412E+05 * t
           - 0.421278819301717E+04 * t * t
           + 0.381476328422343E+03 * t * t * t;
    }

    return bc;
}

// Returns a star's absolute magnitude, given its apparent magnitude (appMag)
// and distance in parsecs (dist). If the distance is zero or infinite, returns infinity.

double SSStar::absoluteMagnitude ( double appMag, double dist )
{
    if ( dist > 0.0 && dist < INFINITY )
        return appMag - 5.0 * ( log10 ( dist ) - 1.0 );
    else
        return -INFINITY;
}

// Returns a star's apparent magnitude, given its absolute magnitude (absMag)
// and distance in parsecs (dist). If the distance is zero or infinite, returns infinity.

double SSStar::apparentMagnitude ( double absMag, double dist )
{
    if ( dist > 0.0 && dist < INFINITY )
        return absMag + 5.0 * ( log10 ( dist ) - 1.0 );
    else
        return dist <= 0.0 ? -INFINITY : INFINITY;
}

// Returns a star's distance in parsecs from the difference between
// its apparent and absolute magnitudes.

double SSStar::distanceFromMagnitude ( double appMag, double absMag )
{
    return pow ( 10.0, ( appMag - absMag ) / 5.0 ) + 1.0;
}

// Returns the brightness ratio that corresponds to the magnitude difference (magDiff)
// between two stars. If magDiff < 0, the ratio is > 1; if magDiff > 0, the ratio is < 1.
// If magDiff positive infinite, the ratio is inifite; if mgative infinite, the ratio is zero.

double SSStar::brightnessRatio ( double magDiff )
{
    if ( isinf ( magDiff ) )
        return magDiff > 0.0 ? INFINITY : 0.0;
    else
        return pow ( 10.0, magDiff / 2.5 );
}

// Given the brightness ratio between two objects, returns their difference in magnitudes.
// If the ratio is < 1, the magnitude difference is postive; if > 1, it is negative.

double SSStar::magnitudeDifference ( double ratio )
{
    return -2.5 * log10 ( ratio );
}

// Returns the combined magnitude of two stars with individual magnitudes
// mag1 and mag2. If either magnitude is infinite, the function returns the
// other magnitude.

double SSStar::magnitudeSum ( double mag1, double mag2 )
{
    if ( isinf ( mag2 ) )
        return mag1;
    else if ( isinf ( mag1 ) )
        return mag2;
    else
        return mag2 + magnitudeDifference ( 1.0 + brightnessRatio ( mag1 - mag2 ) );
}

// A Moffat function describes a stellar image profile on a CCD image:
// https://ned.ipac.caltech.edu/level5/Stetson/Stetson2_2_1.html
// Maximum intensity at center of star image, in arbitrary units, is (max).
// Square of distance in pixels from star image center is (r2).
// Power law exponent (beta) describes rate intensity declines as distance from center increases.
// For real stars beta values from 2.5 to 4.675 are used, see:
// https://www.startools.org/modules/decon/usage/the-point-spread-function-psf

double SSStar::moffatFunction ( double max, double r2, double beta )
{
    return max / pow ( 1.0 + r2, beta );
}

// Computes radius in pixels from center of a Moffat-function star image profile
// where intensity equals a given value (z). Other Moffat function parameters,
// (max) and (beta) are as described above for moffatFunction().

double SSStar::moffatRadius ( double z, double max, double beta )
{
    return sqrt ( pow ( max / z, 1.0 / beta ) - 1.0 );
}

// Given a stellar spectral class string, returns integer code for spectral type.
// Note obsolete types R and N are now considered part of C but are still parsed
// separately since these are found in the SKY2000 Master Star Catalog.
// Assumes leading and trailing whitespace has been removed.
// See https://en.wikipedia.org/wiki/Stellar_classification

int SSStar::spectralType ( const string &spectrum )
{
    int spectype = 0;
    static char types[14] = { 'W', 'O', 'B', 'A', 'F', 'G', 'K', 'M', 'L', 'T', 'R', 'N', 'S', 'C'  };

    for ( int i = 0; i < spectrum.length(); i++ )
    {
        for ( int k = 0; k < 12; k++ )
        {
            if ( spectrum[i] == types[k] )
            {
                spectype = k * 10;
                i++;
                if ( spectrum[i] >= '0' && spectrum[i] <= '9' )
                    spectype += spectrum[i] - '0';
                    
                return spectype;
            }
        }
    }
    
    return 0;
}

// Given a stellar spectral class string, returns integer code for luminosity class.
// Assumes leading and trailing whitespace has been removed.
// See https://en.wikipedia.org/wiki/Stellar_classification

int SSStar::luminosityClass ( const string &spectrum )
{
    int i, lumclass = 0;
    
    // Mt. Wilson luminosity classes c, g, sg, d, sd, and white dwarfs (D) are prefixed.
    
    if ( spectrum[0] == 'c' )
        lumclass = LumClass::Iab;

    if ( spectrum[0] == 'g' )
        lumclass = LumClass::III;
    
    if ( spectrum[0] == 's' && spectrum.size() >= 2 )
    {
        if ( spectrum[1] == 'g' )
            lumclass = LumClass::IV;
        else if ( spectrum[1] == 'd' )
            lumclass = LumClass::VI;
    }

    if ( spectrum[0] == 'd' )
        lumclass = LumClass::V;
    
    if ( spectrum[0] == 'D' )
        lumclass = LumClass::VII;
    
    if ( lumclass > 0 )
        return lumclass;
    
    // Yerkes (Morgan-Keenan) luminosity classes are suffixed after spectral type.
    // Find the first luminosity class character in the string.
    // If none, return zero to indicate unknown luminosity class.
    
    i = (int) spectrum.find_first_of ( "IV" );
    if ( i == string::npos )
        return 0;
    
    // Class V, VI, VII
    
    if ( spectrum.compare ( i, 3, "VII", 3 ) == 0 )
        lumclass = LumClass::VII;
    else if ( spectrum.compare ( i, 2, "VI", 2 ) == 0 )
        lumclass = LumClass::VI;
    else if ( spectrum.compare ( i, 1, "V", 1 ) == 0 )
        lumclass = LumClass::V;

    // Class Ia0, Ia, Iab, Ib

    else if ( spectrum.compare ( i, 3, "Iab", 3 ) == 0 )
        lumclass = LumClass::Iab;
    else if ( spectrum.compare ( i, 3, "IAB", 3 ) == 0 )
        lumclass = LumClass::Iab;
    else if ( spectrum.compare ( i, 3, "Ia0", 3 ) == 0 )
        lumclass = LumClass::Ia0;
    else if ( spectrum.compare ( i, 3, "IA0", 3 ) == 0 )
        lumclass = LumClass::Ia0;
    else if ( spectrum.compare ( i, 3, "IA+", 3 ) == 0 )
        lumclass = LumClass::Ia0;
    else if ( spectrum.compare ( i, 2, "Ia", 2 ) == 0 )
        lumclass = LumClass::Ia;
    else if ( spectrum.compare ( i, 2, "IA", 2 ) == 0 )
        lumclass = LumClass::Ia;
    else if ( spectrum.compare ( i, 2, "Ib", 2 ) == 0 )
        lumclass = LumClass::Ib;
    else if ( spectrum.compare ( i, 2, "IB", 2 ) == 0 )
        lumclass = LumClass::Ib;

    // Class II, III, and IV

    else if ( spectrum.compare ( i, 3, "III", 3 ) == 0 )
        lumclass = LumClass::III;
    else if ( spectrum.compare ( i, 2, "II", 2 ) == 0 )
        lumclass = LumClass::II;
    else if ( spectrum.compare ( i, 2, "IV", 2 ) == 0 )
        lumclass = LumClass::IV;
    
    return lumclass;
}

// Given a stellar spectral class string, parses integer code for spectral type
// and luminosity class. Assumes leading and trailing whitespace has been removed.
// See https://en.wikipedia.org/wiki/Stellar_classification

bool SSStar::parseSpectrum ( const string &spectrum, int &spectype, int &lumclass )
{
    spectype = spectralType ( spectrum );
    lumclass = luminosityClass ( spectrum );
    return spectype || lumclass;
}

// Given an integer spectral type and luminosity class code,
// formats and returns equivalent spectral class string.
// Note obsolete types R and N are now considered part of C.

string SSStar::formatSpectrum ( int spectype, int lumclass )
{
    string spectrum = "";
    static char types[14] = { 'W', 'O', 'B', 'A', 'F', 'G', 'K', 'M', 'L', 'T', 'R', 'N', 'S', 'C'  };
    
    if ( lumclass == LumClass::VII )
        spectrum.append ( 1, 'D' );
    
    if ( spectype > SpecType::W0 && spectype < SpecType::T0 + 9 )
    {
        spectrum.append ( 1, types[ spectype / 10 ] );
        spectrum.append ( 1, '0' + spectype % 10 );
    }
    
    if ( lumclass == LumClass::Ia0 )
        spectrum.append ( "Ia0" );
    else if ( lumclass == LumClass::Ia )
        spectrum.append ( "Ia" );
    else if ( lumclass == LumClass::Iab )
        spectrum.append ( "Iab" );
    else if ( lumclass == LumClass::Ib )
        spectrum.append ( "Ib" );
    else if ( lumclass == LumClass::II )
        spectrum.append ( "II" );
    else if ( lumclass == LumClass::III )
        spectrum.append ( "III" );
    else if ( lumclass == LumClass::IV )
        spectrum.append ( "IV" );
    else if ( lumclass == LumClass::V )
        spectrum.append ( "V" );
    else if ( lumclass == LumClass::VI )
        spectrum.append ( "VI" );

    return spectrum;
}

struct SpecTemp { int spec; float temp; };

float SSStar::spectralTemperature ( int spectype, int lumclass )
{
    static vector<SpecTemp> tempsV =
    {
        { SpecType::O0 + 5, 54000.0f },
        { SpecType::O0 + 6, 45000.0f },
        { SpecType::O0 + 7, 43300.0f },
        { SpecType::O0 + 8, 40600.0f },
        { SpecType::O0 + 9, 37800.0f },
        { SpecType::B0,     29200.0f },
        { SpecType::B0 + 1, 23000.0f },
        { SpecType::B0 + 2, 21000.0f },
        { SpecType::B0 + 3, 17600.0f },
        { SpecType::B0 + 5, 15200.0f },
        { SpecType::B0 + 6, 14300.0f },
        { SpecType::B0 + 7, 13500.0f },
        { SpecType::B0 + 8, 12300.0f },
        { SpecType::B0 + 9, 11400.0f },
        { SpecType::A0,      9600.0f },
        { SpecType::A0 + 1, 9330.0f },
        { SpecType::A0 + 2, 9040.0f },
        { SpecType::A0 + 3, 8750.0f },
        { SpecType::A0 + 4, 8480.0f },
        { SpecType::A0 + 5, 8310.0f },
        { SpecType::A0 + 7, 7920.0f },
        { SpecType::F0,     7350.0f },
        { SpecType::F0 + 2, 7050.0f },
        { SpecType::F0 + 3, 6850.0f },
        { SpecType::F0 + 5, 6700.0f },
        { SpecType::F0 + 6, 6550.0f },
        { SpecType::F0 + 7, 6400.0f },
        { SpecType::F0 + 8, 6300.0f },
        { SpecType::G0,     6050.0f },
        { SpecType::G0 + 1, 5930.0f },
        { SpecType::G0 + 2, 5800.0f },
        { SpecType::G0 + 5, 5660.0f },
        { SpecType::G0 + 8, 5440.0f },
        { SpecType::K0,     5240.0f },
        { SpecType::K0 + 1, 5110.0f },
        { SpecType::K0 + 2, 4960.0f },
        { SpecType::K0 + 3, 4800.0f },
        { SpecType::K0 + 4, 4600.0f },
        { SpecType::K0 + 5, 4400.0f },
        { SpecType::K0 + 7, 4000.0f },
        { SpecType::M0,     3750.0f },
        { SpecType::M0 + 1, 3700.0f },
        { SpecType::M0 + 2, 3600.0f },
        { SpecType::M0 + 3, 3500.0f },
        { SpecType::M0 + 4, 3400.0f },
        { SpecType::M0 + 5, 3200.0f },
        { SpecType::M0 + 6, 3100.0f },
        { SpecType::M0 + 7, 2900.0f },
        { SpecType::M0 + 8, 2700.0f },
        { SpecType::L0,     2600.0f },
        { SpecType::L0 + 3, 2200.0f },
        { SpecType::L0 + 8, 1500.0f },
        { SpecType::T0 + 2, 1400.0f },
        { SpecType::T0 + 6, 1000.0f },
        { SpecType::T0 + 8, 800.0f },
    };

    static vector<SpecTemp> tempsIII =
    {
        { SpecType::G0 + 5, 5010.0f },
        { SpecType::G0 + 8, 4870.0f },
        { SpecType::K0,     4720.0f },
        { SpecType::K0 + 1, 4580.0f },
        { SpecType::K0 + 2, 4460.0f },
        { SpecType::K0 + 3, 4210.0f },
        { SpecType::K0 + 4, 4010.0f },
        { SpecType::K0 + 5, 3780.0f },
        { SpecType::M0,     3660.0f },
        { SpecType::M0 + 1, 3600.0f },
        { SpecType::M0 + 2, 3500.0f },
        { SpecType::M0 + 3, 3300.0f },
        { SpecType::M0 + 4, 3100.0f },
        { SpecType::M0 + 5, 2950.0f },
        { SpecType::M0 + 6, 2800.0f }
    };
    
    static vector<SpecTemp> tempsI =
    {
        { SpecType::B0,     21000.0f },
        { SpecType::B0 + 1, 16000.0f },
        { SpecType::B0 + 2, 14000.0f },
        { SpecType::B0 + 3, 12800.0f },
        { SpecType::B0 + 5, 11500.0f },
        { SpecType::B0 + 6, 11000.0f },
        { SpecType::B0 + 7, 10500.0f },
        { SpecType::B0 + 8, 10000.0f },
        { SpecType::B0 + 9, 9700.0f },
        { SpecType::A0,     9400.0f },
        { SpecType::A0 + 1, 9100.0f },
        { SpecType::A0 + 2, 8900.0f },
        { SpecType::A0 + 5, 8300.0f },
        { SpecType::F0,     7500.0f },
        { SpecType::F0 + 2, 7200.0f },
        { SpecType::F0 + 5, 6800.0f },
        { SpecType::F0 + 8, 6150.0f },
        { SpecType::G0,     5800.0f },
        { SpecType::G0 + 2, 5500.0f },
        { SpecType::G0 + 5, 5100.0f },
        { SpecType::G0 + 8, 5050.0f },
        { SpecType::K0,     4900.0f },
        { SpecType::K0 + 1, 4700.0f },
        { SpecType::K0 + 2, 4500.0f },
        { SpecType::K0 + 3, 4300.0f },
        { SpecType::K0 + 4, 4100.0f },
        { SpecType::K0 + 5, 3750.0f },
        { SpecType::M0,     3660.0f },
        { SpecType::M0 + 1, 3600.0f },
        { SpecType::M0 + 2, 3500.0f },
        { SpecType::M0 + 3, 3300.0f },
        { SpecType::M0 + 4, 3100.0f },
        { SpecType::M0 + 5, 2950.0f },
    };
    
    vector<SpecTemp> &temps = tempsV;
    if ( lumclass <= LumClass::Ib )
        temps = tempsI;
    else if ( lumclass <= LumClass::III )
        temps = tempsIII;
    
    int i;
    for ( i = 0; i < temps.size(); i++ )
        if ( spectype >= temps[i].spec )
            break;
    
    if ( i == temps.size() )
        return 0.0f;
    
    float temp = temps[i].temp;
    if ( i < temps.size() )
    {
        float dt = temps[i+1].temp - temps[i].temp;
        float ds = temps[i+1].spec - temps[i].spec;
        temp += ( spectype - temps[i].spec ) * dt / ds;
    }
    
    return temp;
}

// Returns CSV string from base data (excluding names and identifiers).

string SSStar::toCSV1 ( void )
{
    SSSpherical coords = getFundamentalCoords();
    SSSpherical motion = getFundamentalMotion();
    
    SSHourMinSec ra = coords.lon;
    SSDegMinSec dec = coords.lat;
    double distance = coords.rad;
    
    string csv = SSObject::typeToCode ( _type ) + ",";
    
    csv += ra.toString() + ",";
    csv += dec.toString() + ",";
    
    csv += isnan ( motion.lon ) || isinf ( motion.lon ) ? "," : format ( "%+.5f,", ( motion.lon / 15.0 ).toArcsec() );
    csv += isnan ( motion.lat ) || isinf ( motion.lat ) ? "," : format ( "%+.4f,", motion.lat.toArcsec() );
    
    csv += isinf ( _Vmag ) ? "," : format ( "%+.2f,", _Vmag );
    csv += isinf ( _Bmag ) ? "," : format ( "%+.2f,", _Bmag );
    
    csv += isinf ( distance ) ? "," : format ( "%.3E,", distance * SSCoordinates::kParsecPerLY );
    csv += isinf ( _radvel ) ? "," : format ( "%+.1f,", _radvel * SSCoordinates::kLightKmPerSec );
    
    // If spectrum contains a comma, put it in quotes.
    
    csv += _spectrum.find ( "," ) == string::npos ? _spectrum + "," : "\"" + _spectrum + "\",";
    
    return csv;
}

// Returns CSV string from identifiers and names (excluding base data).

string SSStar::toCSV2 ( void )
{
    string csv = "";
    
    for ( int i = 0; i < _idents.size(); i++ )
        csv += _idents[i].toString() + ",";
    
    for ( int i = 0; i < _names.size(); i++ )
        csv += _names[i] + ",";

    return csv;
}

// Returns CSV string including base star data plus names and identifiers.

string SSStar::toCSV ( void )
{
    return toCSV1() + toCSV2();
}

// Returns CSV string from double-star data (but not SStar base class).

string SSDoubleStar::toCSVD ( void )
{
    string csv = "";
    
    csv += _comps + ",";
    csv += isinf ( _magDelta ) ? "," : format ( "%+.2f,", _magDelta );
    csv += isinf ( _sep ) ? "," : format ( "%.1f,", _sep * SSAngle::kArcsecPerRad );
    csv += isinf ( _PA ) ? "," : format ( "%.1f,", _PA * SSAngle::kDegPerRad );
    csv += isinf ( _PAyr ) ? "," : format ( "%.2f,", _PAyr );

    return csv;
}

// Returns CSV string including base star data, double-star data,
// plus names and identifiers. Overrides SSStar::toCSV().

string SSDoubleStar::toCSV ( void )
{
    return toCSV1() + toCSVD() + toCSV2();
}

// Returns CSV string from variable-star data (but not SStar base class).

string SSVariableStar::toCSVV ( void )
{
    string csv = "";
    
    csv += _varType + ",";
    csv += isinf ( _varMinMag ) ? "," : format ( "%+.2f,", _varMinMag );
    csv += isinf ( _varMaxMag ) ? "," : format ( "%+.2f,", _varMaxMag );
    csv += isinf ( _varPeriod ) ? "," : format ( "%.2f,", _varPeriod );
    csv += isinf ( _varEpoch )  ? "," : format ( "%.2f,", _varEpoch );

    return csv;
}

// Returns CSV string including base star data, variable-star data, plus names and identifiers.
// Overrides SSStar::toCSV().

string SSVariableStar::toCSV ( void )
{
    return toCSV1() + toCSVV() + toCSV2();
}

// Returns CSV string including base star data, double-star data, variable-star data,
// plus names and identifiers.  Overrides SSStar::toCSV().

string SSDoubleVariableStar::toCSV ( void )
{
    return toCSV1() + toCSVD() + toCSVV() + toCSV2();
}

// Returns CSV string from deep sky object data (but not SStar base class).

string SSDeepSky::toCSVDS ( void )
{
    string csv = "";

    csv += isinf ( _majAxis ) ? "," : format ( "%.2f,", _majAxis * SSAngle::kArcminPerRad );
    csv += isinf ( _minAxis ) ? "," : format ( "%.2f,", _minAxis * SSAngle::kArcminPerRad );
    csv += isinf ( _PA ) ? "," : format ( "%.1f,", _PA * SSAngle::kDegPerRad );

    return csv;
}

// Returns CSV string including base star data, double-star data,
// plus names and identifiers. Overrides SSStar::toCSV().

string SSDeepSky::toCSV ( void )
{
    if ( _type == kTypeStar )
        return toCSV1() + toCSV2();
    else
        return toCSV1() + toCSVDS() + toCSV2();
}

// Allocates a new SSStar and initializes it from a CSV-formatted string.
// Returns nullptr on error (invalid CSV string, heap allocation failure, etc.)

SSObjectPtr SSStar::fromCSV ( string csv )
{
    // split string into comma-delimited fields,
    // remove leading & trailing whitespace from each field.
    
    vector<string> fields = split_csv ( csv );
    for ( int i = 0; i < fields.size(); i++ )
        fields[i] = trim ( fields[i] );
    
    SSObjectType type = SSObject::codeToType ( fields[0] );
    if ( type < kTypeStar || type > kTypeGalaxy )
        if ( type != kTypeNonexistent )
            return nullptr;
    
    // Set expected field index for first identifier based on object type.
    // Verify that we have the required number if fiels and return if not.
    
    int fid = 0;
    if ( type == kTypeStar )
        fid = 10;
    else if ( type == kTypeDoubleStar )
        fid = 15;
    else if ( type == kTypeVariableStar )
        fid = 15;
    else if ( type == kTypeDoubleVariableStar )
        fid = 20;
    else
        fid = 13;
    
    if ( fields.size() < fid )
        return nullptr;
    
    SSHourMinSec ra ( fields[1] );
    SSDegMinSec dec ( fields[2] );
    
    double pmRA = fields[3].empty() ? INFINITY : SSAngle::kRadPerArcsec * strtofloat64 ( fields[3] ) * 15.0;
    double pmDec = fields[4].empty() ? INFINITY : SSAngle::kRadPerArcsec * strtofloat64 ( fields[4] );
    
    float vmag = fields[5].empty() ? INFINITY : strtofloat ( fields[5] );
    float bmag = fields[6].empty() ? INFINITY : strtofloat ( fields[6] );
    
    float dist = fields[7].empty() ? INFINITY : strtofloat ( fields[7] ) * SSCoordinates::kLYPerParsec;
    float radvel = fields[8].empty() ? INFINITY : strtofloat ( fields[8] ) / SSCoordinates::kLightKmPerSec;
    string spec = trim ( fields[9] );
    
    // For remaining fields, attempt to parse an identifier.
    // If we succeed, add it to the identifier vector; otherwise add it to the name vector.
    
    vector<string> names;
    vector<SSIdentifier> idents;
    
    for ( int i = fid; i < fields.size(); i++ )
    {
        if ( fields[i].empty() )
            continue;
        
        SSIdentifier ident = SSIdentifier::fromString ( fields[i] );
        if ( ident )
            idents.push_back ( ident );
        else
            names.push_back ( fields[i] );
    }
    
    SSObjectPtr pObject = SSNewObject ( type );
    SSStarPtr pStar = SSGetStarPtr ( pObject );
    SSDoubleStarPtr pDoubleStar = SSGetDoubleStarPtr ( pObject );
    SSVariableStarPtr pVariableStar = SSGetVariableStarPtr ( pObject );
    SSDeepSkyPtr pDeepSkyObject = SSGetDeepSkyPtr ( pObject );

    if ( pStar == nullptr )
        return nullptr;

    SSSpherical coords ( ra, dec, dist );
    SSSpherical motion ( pmRA, pmDec, radvel );
    
    pStar->setFundamentalMotion ( coords, motion );
    pStar->setVMagnitude ( vmag );
    pStar->setBMagnitude ( bmag );
    pStar->setSpectralType ( spec );
    pStar->setIdentifiers( idents );
    pStar->setNames ( names );
    
    if ( pDoubleStar )
    {
        string comps = fields[10];
        float dmag = fields[11].empty() ? INFINITY : strtofloat ( fields[11] );
        float sep = fields[12].empty() ? INFINITY : strtofloat ( fields[12] ) / SSAngle::kArcsecPerRad;
        float pa = fields[13].empty() ? INFINITY : strtofloat ( fields[13] ) / SSAngle::kDegPerRad;
        float year = fields[14].empty() ? INFINITY : strtofloat ( fields[14] );

        pDoubleStar->setComponents ( comps );
        pDoubleStar->setMagnitudeDelta( dmag );
        pDoubleStar->setSeparation ( sep );
        pDoubleStar->setPositionAngle ( pa );
        pDoubleStar->setPositionAngleYear ( year );
    }

    if ( pVariableStar )
    {
        int fv = ( type == kTypeVariableStar ) ? 10 : 15;
            
        string vtype = fields[fv];
        float vmin = fields[fv+1].empty() ? INFINITY : strtofloat ( fields[fv+1] );
        float vmax = fields[fv+2].empty() ? INFINITY : strtofloat ( fields[fv+2] );
        float vper = fields[fv+3].empty() ? INFINITY : strtofloat ( fields[fv+3] );
        double vep = fields[fv+4].empty() ? INFINITY : strtofloat64 ( fields[fv+4] );
        
        pVariableStar->setVariableType ( vtype );
        pVariableStar->setMaximumMagnitude ( vmax );
        pVariableStar->setMinimumMagnitude ( vmin );
        pVariableStar->setPeriod ( vper );
        pVariableStar->setEpoch ( vep );
    }
    
    if ( pDeepSkyObject )
    {
        float major = fields[10].empty() ? INFINITY : strtofloat ( fields[10] ) / SSAngle::kArcminPerRad;
        float minor = fields[11].empty() ? INFINITY : strtofloat ( fields[11] ) / SSAngle::kArcminPerRad;
        float pa = fields[12].empty() ? INFINITY : strtofloat ( fields[12] ) / SSAngle::kDegPerRad;
        
        pDeepSkyObject->setMajorAxis ( major );
        pDeepSkyObject->setMinorAxis ( minor );
        pDeepSkyObject->setPositionAngle ( pa );
    }
    
    return ( pObject );
}

// Downcasts generic SSObject pointer to SSStar pointer.
// Returns nullptr if pointer is not an instance of SSStar!

SSStarPtr SSGetStarPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSStarPtr> ( ptr );
}

// Downcasts generic SSObject pointer to SSDoubleStar pointer.
// Returns nullptr if pointer is not an instance of SSDoubleStar
// or SSDoubleVariableStar!

SSDoubleStarPtr SSGetDoubleStarPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSDoubleStarPtr> ( ptr );
}

// Downcasts generic SSObject pointer to SSVariableStar pointer.
// Returns nullptr if pointer is not an instance of SSVariableStar
// or SSDoubleVariableStar!

SSVariableStarPtr SSGetVariableStarPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSVariableStarPtr> ( ptr );
}

// Downcasts generic SSObject pointer to SSDeepSkyStar pointer.
// Returns nullptr if pointer is not an instance of SSDeepSky!

SSDeepSkyPtr SSGetDeepSkyPtr ( SSObjectPtr ptr )
{
    return dynamic_cast<SSDeepSkyPtr> ( ptr );
}
