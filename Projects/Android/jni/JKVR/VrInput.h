
#if !defined(vrinput_h)
#define vrinput_h

#include "VrCommon.h"

#define STABILISATION_DISTANCE   0.5
#define SCOPE_ENGAGE_DISTANCE   0.25
#define VSTOCK_ENGAGE_DISTANCE   0.25
#define BINOCULAR_ENGAGE_DISTANCE   0.25
#define VELOCITY_TRIGGER        1.6

extern ovrInputStateTrackedRemote leftTrackedRemoteState_old;
extern ovrInputStateTrackedRemote leftTrackedRemoteState_new;
extern ovrTracking leftRemoteTracking_new;
extern ovrInputStateTrackedRemote rightTrackedRemoteState_old;
extern ovrInputStateTrackedRemote rightTrackedRemoteState_new;
extern ovrTracking rightRemoteTracking_new;
extern ovrInputStateGamepad footTrackedRemoteState_old;
extern ovrInputStateGamepad footTrackedRemoteState_new;
extern ovrDeviceID controllerIDs[2];

extern float remote_movementSideways;
extern float remote_movementForward;
extern float remote_movementUp;
extern float positional_movementSideways;
extern float positional_movementForward;

void sendButtonAction(const char* action, long buttonDown);
void sendButtonActionSimple(const char* action);

void acquireTrackedRemotesData(ovrMobile *Ovr, double displayTime);

void HandleInput_Default( ovrInputStateGamepad *pFootTrackingNew, ovrInputStateGamepad *pFootTrackingOld,
                          ovrInputStateTrackedRemote *pDominantTrackedRemoteNew, ovrInputStateTrackedRemote *pDominantTrackedRemoteOld, ovrTracking* pDominantTracking,
                          ovrInputStateTrackedRemote *pOffTrackedRemoteNew, ovrInputStateTrackedRemote *pOffTrackedRemoteOld, ovrTracking* pOffTracking,
                          int domButton1, int domButton2, int offButton1, int offButton2 );

void HandleInput_WeaponAlign( ovrInputStateTrackedRemote *pDominantTrackedRemoteNew, ovrInputStateTrackedRemote *pDominantTrackedRemoteOld, ovrTracking* pDominantTracking,
                          ovrInputStateTrackedRemote *pOffTrackedRemoteNew, ovrInputStateTrackedRemote *pOffTrackedRemoteOld, ovrTracking* pOffTracking,
                          int domButton1, int domButton2, int offButton1, int offButton2 );


void CalculateShoulderPosition();
void updateScopeAngles();

#endif //vrinput_h