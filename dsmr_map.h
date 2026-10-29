/*
  The main readout JSON file. Adopt it for your needs. Items can be removed from it.
  OBIS represents code and Name is name of MQTT topic.
  The code for finding this in the telegram see
    https://www.eso.lt/download/523006/p1%20s%C4%85sajos%20duomen%C5%B3%20modelis.pdf

  UPDATE 2024-05: Lithuanian ESO has introduced standard, extended and maximum plans with
  appropriately assigned readouts. Below is the list for extended plan according to ESO best 
  suited for solar plant owners. Here is the link to readout-plan assignment 
    https://www.eso.lt/download/585550/p1%20duomenu%20modelis_v2.xlsx
*/

const char* DSMR_MAP = R""""(
[
  {
    "OBIS": "0-0:96.7.21",
    "Name": "failures_in_phases"
  },
  {
    "OBIS": "1-0:1.8.0",
    "Name": "active_energy_import"
  },
  {
    "OBIS": "1-0:1.8.1",
    "Name": "active_energy_import_rate_1"
  },
  {
    "OBIS": "1-0:1.8.2",
    "Name": "active_energy_import_rate_2"
  },
  {
    "OBIS": "1-0:1.7.0",
    "Name": "instantaneous_active_import_power"
  },
  {
    "OBIS": "1-0:21.7.0",
    "Name": "instantaneous_active_import_power_in_phase_l1"
  },
  {
    "OBIS": "1-0:41.7.0",
    "Name": "instantaneous_active_import_power_in_phase_l2"
  },
  {
    "OBIS": "1-0:61.7.0",
    "Name": "instantaneous_active_import_power_in_phase_l3"
  },
    {
    "OBIS": "1-0:22.7.0",
    "Name": "instantaneous_active_export_power_in_phase_l1"
  },
  {
    "OBIS": "1-0:42.7.0",
    "Name": "instantaneous_active_export_power_in_phase_l2"
  },
  {
    "OBIS": "1-0:62.7.0",
    "Name": "instantaneous_active_export_power_in_phase_l3"
  },
  {
    "OBIS": "1-0:32.7.0",
    "Name": "instantaneous_voltage_l1"
  },
  {
    "OBIS": "1-0:52.7.0",
    "Name": "instantaneous_voltage_l2"
  },
  {
    "OBIS": "1-0:72.7.0",
    "Name": "instantaneous_voltage_l3"
  },
  {
    "OBIS": "1-0:13.7.0",
    "Name": "instantaneous_power_factor"
  },
  {
    "OBIS": "1-0:2.8.0",
    "Name": "active_energy_export"
  },
  {
    "OBIS": "1-0:2.7.0",
    "Name": "instantaneous_active_export_power"
  }
]
)"""";
