// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "SAttributeComponent.h"
#include "SInteractionComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//using default function template initiate the pointers
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	//attaching SpringArmComp to the root of the character which is the camera
	SpringArmComp->SetupAttachment(RootComponent);
	//setup pawn control here - can also do it in blueprint
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	//Attach the CameraComp to the SpringArmComp, cuz we want third person view
	CameraComp->SetupAttachment(SpringArmComp);

	//instantiate the character interaction behavior (and then bind in input component)
	InteractionComp = CreateDefaultSubobject<USInteractionComponent>("InteractionComp");

	//initialize attribute comp
	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

	//rotate the character towards the direction we facing
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//set the yaw movement to false - for a rpg style controller camera
	bUseControllerRotationYaw = false;

	//delay the spawn of attack projectile, set this up for avoiding hard coding
	AttackAnimDelay = 0.2f;

	TimeToHitParamName = "TimeToHit";
	HandSocketName = "Muzzle_01";
}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	AttributeComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}

//set the view point to the camera instead of character eyes
FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComp->GetComponentLocation();
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//bind player movement to axis, 3 parameters: name, obj, and move function
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	//bind pawn control movement (like moving a camera)
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	//bind an input action - if the key is pressed instead of an input of value
	PlayerInputComponent->BindAction("PrimaryAttack", IE_Pressed, this, &ASCharacter::PrimaryAttack);
	PlayerInputComponent->BindAction("SecondaryAttack", IE_Pressed, this, &ASCharacter::BlackHoleAttack);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASCharacter::Dash);

	//bind the interact action - E key
	PlayerInputComponent->BindAction("PrimaryInteract", IE_Pressed, this, &ASCharacter::PrimaryInteract);

	//Directly use built-in jump function in ACharacter.h
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
}

/*
// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//--Rotation Visualization for debug purpose only
	const float DrawScale = 100.0f;
	const float Thickness = 5.0f;

	FVector LineStart = GetActorLocation();
	//offset to the right of pawn
	LineStart += GetActorRightVector() * 100.0f;
	//set line end in direction of the actor's forward
	FVector ActorDirection_LineEnd = LineStart + (GetActorForwardVector() * 100.0f);
	//draw Actor's Direction
	DrawDebugDirectionalArrow(GetWorld(), LineStart, ActorDirection_LineEnd, DrawScale, FColor::Yellow, false, 0.0f, 0, Thickness);

	FVector ControllerDirection_LineEnd = LineStart + (GetControlRotation().Vector() * 100.0f);
	// Draw 'Controller' Rotation ('PlayerController' that 'possessed' this character)
	DrawDebugDirectionalArrow(GetWorld(), LineStart, ControllerDirection_LineEnd, DrawScale, FColor::Green, false, 0.0f, 0, Thickness);
	
}
*/

void ASCharacter::HealSelf(float Amount /* = 100.0f*/)
{
	AttributeComp->ApplyHealthChange(this, Amount);
}

void ASCharacter::MoveForward(float value)
{
	//fix the movement towards the direction of the rotator
	FRotator ControlRot = GetControlRotation();
	//rotator only affects yaw direction
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	//using built in function from Character class, passing rotator and return to vector
	AddMovementInput(ControlRot.Vector(), value);
}

void ASCharacter::MoveRight(float value)
{
	//fix the movement towards the direction of the rotator
	FRotator ControlRot = GetControlRotation();
	//rotator only affects yaw direction
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	//UE axis: x = forward (red), y = right(green), z = up(blue)
	FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	//using built in function from Character class
	AddMovementInput(RightVector, value);
}

void ASCharacter::PrimaryAttack()
{
	//connect to the exsiting attack animations from contents, still needs to setup in blueprint
	PlayAnimMontage(AttackAnim);

	//use a timer to delay the spawning of projectile to synchronize it with attack animation
	//A handler timer is a struct to hold the handle of this timer, pass as first parameter when set timer
	//PrimaryAttack_Elapsed as a functor, which actually implements the spawning of the projectile, after 0.2s
	GetWorldTimerManager().SetTimer(TimerHandle_PrimaryAttack, this, &ASCharacter::PrimaryAttack_Elapsed, AttackAnimDelay);

	//GetWorldTimerManager().ClearTimer(TimerHandle_PrimaryAttack); //cancel the delay/timer immediately
}

void ASCharacter::PrimaryAttack_Elapsed()
{

	SpawnProjectile(ProjectileClass);

	/*
	//--- moved into SpawnProjectile method which is more uniformed 
	//find a specific location in the mesh - in this case, right hand
	FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");
	//transform is a struct holds a location, rotation, and scale. here we pass in rotation and location
	FTransform SpawnTM = FTransform(GetControlRotation(), HandLocation);

	//FActor is a struct holds a ton of parameters to use
	FActorSpawnParameters SpawnParams;
	//this handler allows us to specify spawn rule through this SpawnCollision variable
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	//Instigator is a built-in variable that equals to the charater who spawns this projectile
	SpawnParams.Instigator = this;

	//use GetWorld() to spawn stuff
	GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
	*/
}

void ASCharacter::BlackHoleAttack()
{
	PlayAnimMontage(AttackAnim);

	GetWorldTimerManager().SetTimer(TimerHandle_BlackholeAttack, this, &ASCharacter::BlackholeAttack_TimeElapsed, AttackAnimDelay);
}

void ASCharacter::BlackholeAttack_TimeElapsed()
{
	SpawnProjectile(BlackHoleProjectileClass);
}

void ASCharacter::Dash()
{
	PlayAnimMontage(AttackAnim);

	GetWorldTimerManager().SetTimer(TimerHandle_Dash, this, &ASCharacter::Dash_TimeElapsed, AttackAnimDelay);
}

void ASCharacter::Dash_TimeElapsed()
{
	SpawnProjectile(DashProjectileClass);
}

void ASCharacter::StartAttackEffects()
{
	PlayAnimMontage(AttackAnim);
	//casting effects
	UGameplayStatics::SpawnEmitterAttached(CastingEffect, GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
}

void ASCharacter::SpawnProjectile(TSubclassOf<AActor> ClassToSpawn)
{
	if (ensureAlways(ClassToSpawn))
	{
		//find a specific location in the mesh - in this case, right hand
		FVector HandLocation = GetMesh()->GetSocketLocation(HandSocketName);

		//FActor is a struct holds a ton of parameters to use
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;

		FCollisionShape Shape;
		Shape.SetSphere(20.0f);

		// Ignore Player
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		//sweeping/looking for object types below
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);

		FVector TraceStart = CameraComp->GetComponentLocation();
		// endpoint far into the look-at distance (not too far, still adjust somewhat towards crosshair on a miss)
		FVector TraceEnd = CameraComp->GetComponentLocation() + (GetControlRotation().Vector() * 5000);

		FHitResult Hit;

		
		// true if we got to a blocking hit (Alternative: SweepSingleByChannel with ECC_WorldDynamic)
		if (GetWorld()->SweepSingleByObjectType(Hit, TraceStart, TraceEnd, FQuat::Identity, ObjParams, Shape, Params))
		{
			// Overwrite trace end with impact point in world
			TraceEnd = Hit.ImpactPoint;
		}

		// find new direction/rotation from Hand pointing to impact point in world.
		FRotator ProjRotation = FRotationMatrix::MakeFromX(TraceEnd - HandLocation).Rotator();


		//transform is a struct holds a location, rotation, and scale. here we pass in rotation and location
		FTransform SpawnTM = FTransform(ProjRotation, HandLocation);
		GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnTM, SpawnParams);
	}
}

void ASCharacter::PrimaryInteract()
{
	//safety check but unnecessary
	if (InteractionComp)
	{
		//same name function but it is built-in SInteractionComponent class
		InteractionComp->PrimaryInteract();
	}
}

void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		//HitFlash component
		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName, GetWorld()->TimeSeconds);
	}

	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		DisableInput(PC);
	}
}

