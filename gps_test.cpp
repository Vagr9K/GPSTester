#include <pthread.h>
#include <signal.h>
#include <hardware/gps.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <cstdlib>

const GpsInterface* gGpsInterface = NULL;
const AGpsInterface* gAGpsInterface = NULL;
const AGpsRilInterface* gAGpsRilInterface = NULL;

static const GpsInterface* get_gps_interface()
{
  int err;
  hw_module_t* module;
  const GpsInterface* interface = NULL;

  fprintf(stdout, "* Acquire GPS module\n");
  err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);

  if (!err) {
    hw_device_t* device;
    fprintf(stdout, "** Acquire GPS device\n");
    err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);
    if (!err) {
      gps_device_t* gps_device = (gps_device_t *)device;
      fprintf(stdout, "*** Acquire GPS interface\n");
      interface = gps_device->get_gps_interface(gps_device);
    } else {
      fprintf(stdout, "*** FAILED acquiring GPS device\n");
    }
  } else {
    fprintf(stdout, "* FAILED acquiring GPS module\n");
    if(err == -ENOENT){
      fprintf(stdout, "-----> MODULE NOT LISTED\n");
    }
    if(err == -EINVAL){
      fprintf(stdout, "-----> DLLOAD FAILED\n");
    }
    fprintf(stdout, "-----> ERROR: %d", err);
  }
  return interface;
}

static const AGpsInterface* get_agps_interface(const GpsInterface *gps)
{
  const AGpsInterface* interface = NULL;

  if (gps) {
    interface = (const AGpsInterface*)gps->get_extension(AGPS_INTERFACE);
  }

  return interface;
}

static const AGpsRilInterface* get_agps_ril_interface(const GpsInterface *gps)
{
  const AGpsRilInterface* interface = NULL;

  if (gps) {
    interface = (const AGpsRilInterface*)gps->get_extension(AGPS_RIL_INTERFACE);
  }

  return interface;
}

static void location_callback(GpsLocation* location)
{
  fprintf(stdout, "@@@ LOCATION_CALLBACK\n");
  fprintf(stdout, "@@@@ flags:\t%d\n", location->flags);
  fprintf(stdout, "@@@@ lat:  \t%lf\n", location->latitude);
  fprintf(stdout, "@@@@ long: \t%lf\n", location->longitude);
  fprintf(stdout, "@@@@ accur:\t%f\n", location->accuracy);
  fprintf(stdout, "@@@@ utc:  \t%ld\n", (long)location->timestamp);
}

static void status_callback(GpsStatus* status)
{
  fprintf(stdout, "@@@ GPS_STATUS_CALLBACK\n");
  switch (status->status) {
  case GPS_STATUS_NONE:
    fprintf(stdout, "@@@@ no gps\n");
    break;
  case GPS_STATUS_SESSION_BEGIN:
    fprintf(stdout, "@@@@ session begin\n");
    break;
  case GPS_STATUS_SESSION_END:
    fprintf(stdout, "@@@@ session end\n");
    break;
  case GPS_STATUS_ENGINE_ON:
    fprintf(stdout, "@@@@ engine on\n");
    break;
  case GPS_STATUS_ENGINE_OFF:
    fprintf(stdout, "@@@@ engine off\n");
    break;
  default:
    fprintf(stdout, "@@@@ unknown status\n");
  }
}

static void sv_status_callback(GpsSvStatus* sv_info)
{
  fprintf(stdout, "@@@ SV_STATUS_CALLBACK\n");
  fprintf(stdout, "@@@@ sv info\n");
  fprintf(stdout, "@@@@ num_svs:\t%ud\n", sv_info->size);
}

static void nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
  fprintf(stdout, "@@@ NMEA_STATUS_CALLBACK\n");
  fprintf(stdout, "@@@@ nmea info\n");
  fprintf(stdout, "@@@@ timestamp:\t%ld\n", (long)timestamp);
  fprintf(stdout, "@@@@ nmea:     \t%s", nmea);
  fprintf(stdout, "@@@@ length:   \t%d\n", length);
}

static void set_capabilities_callback(uint32_t capabilities)
{
  fprintf(stdout, "@@@ SET_CAPABILITIES_CALLBACK\n");
}

static void acquire_wakelock_callback()
{
  fprintf(stdout, "@@@ ACQUIRE_WAKELOCK_CALLBACK\n");
}

static void release_wakelock_callback()
{
  fprintf(stdout, "@@@ RELEASE_WAKELOCK_CALLBACK\n");
}

static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
  fprintf(stdout, "@@@ THREAD_STATUS_CALLBACK\n");
  pthread_t thread;
  pthread_attr_t attr;
  int err;

  err = pthread_attr_init(&attr);
  err = pthread_create(&thread, &attr, (void*(*)(void*))start, arg);

  return thread;
}

GpsCallbacks gps_cb = {
  sizeof(GpsCallbacks),
  location_callback,
  status_callback,
  sv_status_callback,
  nmea_callback,
  set_capabilities_callback,
  acquire_wakelock_callback,
  release_wakelock_callback,
  create_thread_callback,
};

static void
agps_status_cb(AGpsStatus *status)
{
  fprintf(stdout, "@@@ AGPS_STATUS_CALLBACK\n");
  switch (status->status) {
  case GPS_REQUEST_AGPS_DATA_CONN:
    fprintf(stdout, "@@@@ data_conn_open\n");
    gAGpsInterface->data_conn_open("internet");
    break;
  case GPS_RELEASE_AGPS_DATA_CONN:
    fprintf(stdout, "@@@@ data_conn_closed\n");
    gAGpsInterface->data_conn_closed();
    break;
  }
}

AGpsCallbacks agps_cb = {
  agps_status_cb,
  create_thread_callback,
};

static void
agps_ril_set_id_cb(uint32_t flags)
{
  fprintf(stdout, "@@@ SET_IB_CALLBACK\n");
  gAGpsRilInterface->set_set_id(AGPS_SETID_TYPE_IMSI, "000000000000000");
}

static void
agps_ril_refloc_cb(uint32_t flags)
{
  fprintf(stdout, "@@@ AGPS_RIL_REFLOC_CALLBACK\n");
  AGpsRefLocation location;
}

AGpsRilCallbacks agpsril_cb = {
  agps_ril_set_id_cb,
  agps_ril_refloc_cb,
  create_thread_callback,
};

void sigint_handler(int signum)
{
  fprintf(stdout, "@@@ Cleanup\n");
  if (gGpsInterface) {
    gGpsInterface->stop();
    gGpsInterface->cleanup();
  }
}

int main(int argc, char *argv[])
{
  fprintf(stdout, "Setup signal_handler\n");
  signal(SIGINT, sigint_handler);

  fprintf(stdout, "Acquire GPS interface\n");
  gGpsInterface = get_gps_interface();
  if(!gGpsInterface){
    fprintf(stdout, "FAILED acquiring GPS interface\n");
  }

  fprintf(stdout, "Init GPS interface callbacks\n");
  int CallbackInitStatus = gGpsInterface->init(&gps_cb);
  fprintf(stdout, "--> init(): %d\n", CallbackInitStatus);

  if (gGpsInterface && CallbackInitStatus == 0) {
    fprintf(stdout, "* Initial check passed\n");

    fprintf(stdout, "* Acquire AGPS interface\n");
    gAGpsInterface = get_agps_interface(gGpsInterface);
    if (gAGpsInterface) {
      fprintf(stdout, "** Acquired AGPS interface\n");
      gAGpsInterface->init(&agps_cb);
      gAGpsInterface->set_server(AGPS_TYPE_SUPL, "supl.google.com", 7276);
    }

    fprintf(stdout, "* Acquire RIL interface\n");
    gAGpsRilInterface = get_agps_ril_interface(gGpsInterface);
    if (gAGpsRilInterface) {
      fprintf(stdout, "** Acquired RIL interface\n");
      gAGpsRilInterface->init(&agpsril_cb);
    }

    fprintf(stdout, "** Delete GPS data track\n");
    gGpsInterface->delete_aiding_data(GPS_DELETE_ALL);
    fprintf(stdout, "** START GPS interface\n");
    gGpsInterface->start();
    fprintf(stdout, "** Set GPS position mode\n");
    gGpsInterface->set_position_mode(GPS_POSITION_MODE_MS_BASED,
                                     GPS_POSITION_RECURRENCE_PERIODIC,
                                     1000, 0, 0);
  }
 quit:
  fprintf(stdout, "-> QUIT INITIATED\n");
  sleep(10);
  fprintf(stdout, "-> QUIT\n");
  return 0;
}
