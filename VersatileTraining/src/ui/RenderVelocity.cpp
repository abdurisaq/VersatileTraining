#include "pch.h"
#include "src/core/VersatileTraining.h"

void VersatileTraining::RenderVelocityOfCar(CanvasWrapper canvas) {

	
	if ((isInTrainingPack() || isInTrainingEditor()) &&(currentShotState.extendedStartingVelocity != Vector(0, 0, 0) || currentShotState.startingVelocity != 0.f)) {
		//in a training pack/editor
		
        Vector velocity= Vector(0, 0, 0);
        //reverse getting the velocity from the car rotation
        if (currentShotState.extendedStartingVelocity != Vector(0, 0, 0)) {

            velocity = currentShotState.extendedStartingVelocity;
        }

        else if (currentShotState.startingVelocity != 0.f) {
          
            float pitch_deg = static_cast<float>(currentShotState.carRotation.Pitch) * (90.0f / 16384.0f);
            float yaw_deg = static_cast<float>(currentShotState.carRotation.Yaw) * (360.0f / 65536.0f);

           
            float pitch_rad = (float)(pitch_deg * (PI / 180.0f));
            float yaw_rad = (float)(yaw_deg * (PI / 180.0f));

           
            float x = cosf(pitch_rad) * cosf(yaw_rad);
            float y = cosf(pitch_rad) * sinf(yaw_rad);
            float z = sinf(pitch_rad);

           
            Vector direction = Vector(x, y, z);
            velocity = direction * (float)currentShotState.startingVelocity;

        }
        
        
        
             			
		

        
        Vector start = currentShotState.carLocation;

       
        if (velocity == Vector(0, 0, 0)) {
            return;
        }
        CameraWrapper cam = gameWrapper->GetCamera();
        if (cam.IsNull()) return;
        Vector cameraLocation = cam.GetLocation();
        RT::Frustum frust{ canvas, cam };
            

       
        const Vector gravity = Vector(0, 0, -650.f); 
        const int numSteps = 20;
        const float dt = 0.05f;

        for (int i = 0; i < numSteps; ++i) {
            float t = i * dt;

            Vector pos = start + velocity * t + 0.5f * gravity * t * t;

            if (pos.Z <= 0.f) {
                pos.Z = 0.f;
            }

            float alpha = 1.0f - static_cast<float>(i) / numSteps;
            float radius = 10.f;

            RT::Sphere sphere(pos, radius);
            canvas.SetColor((char)255,(char)255, (char)255, static_cast<char>(alpha * 255));
            sphere.Draw(canvas, frust, cameraLocation, 12); 
        }
		
		

    }
    
    
	
}
