#ifndef MQTT_HOMEASSISTANTUNITS_H
#define MQTT_HOMEASSISTANTUNITS_H

enum class HaUnit : uint8_t
{
    NONE,

    // Power
    POWER_WATT,
    POWER_KILO_WATT,

    // Voltage
    VOLT,

    // Energy
    ENERGY_WATT_HOUR,
    ENERGY_KILO_WATT_HOUR,

    // Electrical
    ELECTRICAL_CURRENT_AMPERE,
    ELECTRICAL_VOLT_AMPERE,

    // Degree
    DEGREE,

    // Currency
    CURRENCY_EURO,
    CURRENCY_DOLLAR,
    CURRENCY_CENT,

    // Temperature
    TEMP_CELSIUS,
    TEMP_FAHRENHEIT,
    TEMP_KELVIN,

    // Time
    TIME_MICROSECONDS,
    TIME_MILLISECONDS,
    TIME_SECONDS,
    TIME_MINUTES,
    TIME_HOURS,
    TIME_DAYS,
    TIME_WEEKS,
    TIME_MONTHS,
    TIME_YEARS,

    // Length
    LENGTH_MILLIMETERS,
    LENGTH_CENTIMETERS,
    LENGTH_METERS,
    LENGTH_KILOMETERS,
    LENGTH_INCHES,
    LENGTH_FEET,
    LENGTH_YARD,
    LENGTH_MILES,

    // Frequency
    FREQUENCY_HERTZ,
    FREQUENCY_GIGAHERTZ,

    // Pressure
    PRESSURE_PA,
    PRESSURE_HPA,
    PRESSURE_BAR,
    PRESSURE_MBAR,
    PRESSURE_INHG,
    PRESSURE_PSI,

    // Volume
    VOLUME_LITERS,
    VOLUME_MILLILITERS,
    VOLUME_CUBIC_METERS,
    VOLUME_CUBIC_FEET,
    VOLUME_GALLONS,
    VOLUME_FLUID_OUNCE,

    // Volume flow
    VOLUME_FLOW_RATE_CUBIC_METERS_PER_HOUR,
    VOLUME_FLOW_RATE_CUBIC_FEET_PER_MINUTE,

    // Area
    AREA_SQUARE_METERS,

    // Mass
    MASS_GRAMS,
    MASS_KILOGRAMS,
    MASS_MILLIGRAMS,
    MASS_MICROGRAMS,
    MASS_OUNCES,
    MASS_POUNDS,

    // Conductivity
    CONDUCTIVITY,

    // Light
    LIGHT_LUX,

    // UV
    UV_INDEX,

    // Percentage
    PERCENTAGE,

    // Irradiation
    IRRADIATION_WATTS_PER_SQUARE_METER,

    // Precipitation
    PRECIPITATION_MILLIMETERS_PER_HOUR,

    // Concentration
    CONCENTRATION_MICROGRAMS_PER_CUBIC_METER,
    CONCENTRATION_MILLIGRAMS_PER_CUBIC_METER,
    CONCENTRATION_PARTS_PER_CUBIC_METER,
    CONCENTRATION_PARTS_PER_MILLION,
    CONCENTRATION_PARTS_PER_BILLION,

    // Speed
    SPEED_MILLIMETERS_PER_DAY,
    SPEED_INCHES_PER_DAY,
    SPEED_METERS_PER_SECOND,
    SPEED_INCHES_PER_HOUR,
    SPEED_KILOMETERS_PER_HOUR,
    SPEED_MILES_PER_HOUR,

    // Signal
    SIGNAL_STRENGTH_DECIBELS,
    SIGNAL_STRENGTH_DECIBELS_MILLIWATT,

    // Data
    DATA_BITS,
    DATA_KILOBITS,
    DATA_MEGABITS,
    DATA_GIGABITS,
    DATA_BYTES,
    DATA_KILOBYTES,
    DATA_MEGABYTES,
    DATA_GIGABYTES,
    DATA_TERABYTES,
    DATA_PETABYTES,
    DATA_EXABYTES,
    DATA_ZETTABYTES,
    DATA_YOTTABYTES,
    DATA_KIBIBYTES,
    DATA_MEBIBYTES,
    DATA_GIBIBYTES,
    DATA_TEBIBYTES,
    DATA_PEBIBYTES,
    DATA_EXBIBYTES,
    DATA_ZEBIBYTES,
    DATA_YOBIBYTES,

    DATA_RATE_BITS_PER_SECOND,
    DATA_RATE_KILOBITS_PER_SECOND,
    DATA_RATE_MEGABITS_PER_SECOND,
    DATA_RATE_GIGABITS_PER_SECOND,
    DATA_RATE_BYTES_PER_SECOND,
    DATA_RATE_KILOBYTES_PER_SECOND,
    DATA_RATE_MEGABYTES_PER_SECOND,
    DATA_RATE_GIGABYTES_PER_SECOND,
    DATA_RATE_KIBIBYTES_PER_SECOND,
    DATA_RATE_MEBIBYTES_PER_SECOND,
    DATA_RATE_GIBIBYTES_PER_SECOND,
};

inline constexpr const char *HaUnitStrings[] PROGMEM = {
    "",
    "W",
    "kW",
    "V",
    "Wh",
    "kWh",
    "A",
    "VA",
    "°",
    "€",
    "$",
    "¢",
    "°C",
    "°F",
    "K",
    "μs",
    "ms",
    "s",
    "min",
    "h",
    "d",
    "w",
    "m",
    "y",
    "mm",
    "cm",
    "m",
    "km",
    "in",
    "ft",
    "yd",
    "mi",
    "Hz",
    "GHz",
    "Pa",
    "hPa",
    "bar",
    "mbar",
    "inHg",
    "psi",
    "L",
    "mL",
    "m³",
    "ft³",
    "gal",
    "fl. oz.",
    "m³/h",
    "ft³/m",
    "m²",
    "g",
    "kg",
    "mg",
    "µg",
    "oz",
    "lb",
    "µS/cm",
    "lx",
    "UV index",
    "%",
    "W/m²",
    "mm/h",
    "µg/m³",
    "mg/m³",
    "p/m³",
    "ppm",
    "ppb",
    "mm/d",
    "in/d",
    "m/s",
    "in/h",
    "km/h",
    "mph",
    "dB",
    "dBm",
    "bit",
    "kbit",
    "Mbit",
    "Gbit",
    "B",
    "kB",
    "MB",
    "GB",
    "TB",
    "PB",
    "EB",
    "ZB",
    "YB",
    "KiB",
    "MiB",
    "GiB",
    "TiB",
    "PiB",
    "EiB",
    "ZiB",
    "YiB",
    "bit/s",
    "kbit/s",
    "Mbit/s",
    "Gbit/s",
    "B/s",
    "kB/s",
    "MB/s",
    "GB/s",
    "KiB/s",
    "MiB/s",
    "GiB/s",
};

inline const char *haUnitToString(HaUnit u)
{
    return (const char *)pgm_read_ptr(&HaUnitStrings[static_cast<uint8_t>(u)]);
}

#endif // MQTT_HOMEASSISTANTUNITS_H