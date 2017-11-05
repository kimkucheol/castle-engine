/*
  Copyright 2017-2017 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in the "Castle Game Engine" distribution,
  for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
*/

/*
  Apple Game Center integration with Castle Game Engine.

  TODO: Apple example code instructs to do this:

  If the game view does not autorotate during gameplay, please create a new UIViewController that does
  autorotate and present the modal view controller from that. To control where the welcome
  banners slide in from you must set the position of the status bar on rotation. You should do
  this even if the status bar is hidden, to make sure banners and alerts appear oriented correctly.
*/

#import "GameCenterService.h"

@implementation GameCenterService

#pragma mark -
#pragma mark Game Center Support

// Check for the availability of Game Center API.
bool isGameCenterAPIAvailable()
{
    // Check for presence of GKLocalPlayer API.
    Class gcClass = (NSClassFromString(@"GKLocalPlayer"));

    // The device must be running running iOS 4.1 or later.
    NSString *reqSysVer = @"4.1";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    bool osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);

    return (gcClass != nil) && osVersionSupported;
}

#pragma mark -
#pragma mark Signing in / out

const int STATUS_SIGNED_OUT = 0;
const int STATUS_SIGNING_IN = 1;
const int STATUS_SIGNED_IN = 2;
const int STATUS_SIGNING_OUT = 3;

- (void)initialize:(bool)autoStartSignInFlowExplicit saveGames: (bool)saveGames
{
    // TODO: ignore saveGames now

    autoSignIn = [[AutoSignIn alloc] init];
    m_autoStartSignInFlow = autoStartSignInFlowExplicit || [autoSignIn isAutoSignIn];
    if (m_autoStartSignInFlow && m_finishedLaunching) {
        [self requestSignIn:TRUE];
    }
}

- (void)applicationDidFinishLaunchingWithOptions
{
    [super applicationDidFinishLaunchingWithOptions];

    m_finishedLaunching = true;
    if (m_autoStartSignInFlow) {
        [self requestSignIn:TRUE];
    }
}

- (void)setStatus:(int) value
{
    if (status != value) {
        status = value;
        NSString* statusStr = [@(status) stringValue];
        [self messageSend: @[@"game-service-status", statusStr]];

        if (status == STATUS_SIGNED_IN) {
            [autoSignIn setAutoSignIn: true];
        }
    }
}

- (void)requestSignIn:(bool) wantsSignedIn
{
    if ( ( wantsSignedIn && status == STATUS_SIGNED_IN) ||
         (!wantsSignedIn && status == STATUS_SIGNED_OUT))
     {
        NSLog(@"Ignoring requestSignIn call, requested the current state");
        return;
    }

    if (status == STATUS_SIGNING_IN ||
        status == STATUS_SIGNING_OUT)
    {
        // Note that the current iOS implementation actually never has
        // status == STATUS_SIGNING_OUT, since sign-out is always immediate.
        NSLog(@"Ignoring requestSignIn call, we are in the middle of sign-in or sign-out.");
        return;
    }

    if (!isGameCenterAPIAvailable()) {
        // Game Center is not available.
        [self setStatus: STATUS_SIGNED_OUT];
        return;
    }

    if (wantsSignedIn) {
        GKLocalPlayer *localPlayer = [GKLocalPlayer localPlayer];

        if (localPlayer.isAuthenticated) {
            // Player was already signed-in actually.
            // This happens if you sign in, sign out, sign in again:
            // because Game Center does not have any method to disconnect,
            // the player actually stays isAuthenticated after sign-out.
            // Setting the localPlayer.authenticateHandler would not do anything then.
            [self setStatus: STATUS_SIGNED_IN];
        } else {
            [self setStatus: STATUS_SIGNING_IN];

            // Avoid warning
            // "Capturing 'localPlayer' strongly in this block is likely to lead to a retain cycle"
            // See https://stackoverflow.com/questions/7205128/fix-warning-capturing-an-object-strongly-in-this-block-is-likely-to-lead-to-a
            __weak GKLocalPlayer *weakLocalPlayer = localPlayer;

            // See https://developer.apple.com/documentation/gamekit/gklocalplayer/1515399-authenticatehandler
            localPlayer.authenticateHandler = ^(UIViewController *viewController, NSError *error)
            {
                // If there is an error, do not assume local player is not authenticated.
                if (weakLocalPlayer.isAuthenticated) {

                    if (!authenticatedPlayerID ||
                        ![authenticatedPlayerID isEqualToString:localPlayer.playerID]) {
                        // Switching Users
                        if (!achievements ||
                            ![authenticatedPlayerID isEqualToString:localPlayer.playerID]) {
                            // If there is an existing Achievements instance,
                            // replace the existing Achievements object with
                            // a new object, and use it to load the new player's
                            // saved achievements.
                            //
                            // It is not necessary for the previous Achievements
                            // object to writes its data first;
                            // It automatically saves the changes whenever its list of stored
                            // achievements changes.

                            achievements = [[Achievements alloc] init];
                        }
                        [achievements loadStoredAchievements];

                        // Current playerID has changed. Create/Load a game state around the new user.
                        authenticatedPlayerID = weakLocalPlayer.playerID;
                    }

                    [self setStatus: STATUS_SIGNED_IN];
                } else {
                    // No user is logged into Game Center, run without Game Center support or user interface.
                    [self setStatus: STATUS_SIGNED_OUT];
                }
            };
        }
    } else {
        // wantsSignedIn == false in this case.
        // There is no method to disconnect in Game Center,
        // https://developer.apple.com/documentation/gamekit/gklocalplayer,
        // so just pretend we're disconnected.
        [self setStatus: STATUS_SIGNED_OUT];
    }
}

- (void)applicationDidEnterBackground
{
    [super applicationDidEnterBackground];

    /*
      Apple writes that at this point we should:

      Invalidate Game Center Authentication and save game state, so the game doesn't start
      until the Authentication Completion
      Handler is run. This prevents a new user from using the old users game state.

      Doing setStatus to STATUS_SIGNED_OUT is closest to this, as far as I see.
      There's no way to actually tell GKLocalPlayer to disconnect.
    */
    [self setStatus: STATUS_SIGNED_OUT];
}

#pragma mark -
#pragma mark Show Game Center controllers

- (void)showAchievements
{
    // TODO: our Pascal API promises that it automatically signs-in user if not signed-in already
    GKGameCenterViewController* gameCenterController = [[GKGameCenterViewController alloc] init];
    if (gameCenterController != nil) {
        gameCenterController.gameCenterDelegate = self;
        gameCenterController.viewState = GKGameCenterViewControllerStateAchievements;

        [self.mainController presentViewController: gameCenterController animated: YES completion:nil];
    }
}

- (void)showLeaderboards
{
    // TODO: our Pascal API promises that it automatically signs-in user if not signed-in already
    GKGameCenterViewController* gameCenterController = [[GKGameCenterViewController alloc] init];
    if (gameCenterController != nil) {
        gameCenterController.gameCenterDelegate = self;
        gameCenterController.viewState = GKGameCenterViewControllerStateLeaderboards;
        [self.mainController presentViewController: gameCenterController animated: YES completion:nil];
    }
}

/* Called when gameCenterController is closed. */
- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController *)gameCenterViewController
{
    UIViewController *presentedViewController = self.mainController.presentedViewController;
    if (presentedViewController != nil) {
        [presentedViewController dismissViewControllerAnimated:YES completion:nil];
    }
}

#pragma mark -
#pragma mark Messages

- (void)achievement:(NSString* )achievementId
{
    if (status == STATUS_SIGNED_IN) {
        [achievements submitAchievement:achievementId];
    }
}

/* Make a log, and messageSend, that loading savegame failed. */
- (void)saveGameLoadingError:(NSString* )errorStr
{
    NSLog(@"Saving game failed: %@", errorStr);
    [self messageSend:@[@"save-game-loaded", @"false", errorStr]];
}

- (void)saveGameLoad:(NSString*) saveGameName
{
    if (status == STATUS_SIGNED_IN) {
        // TODO
        [self saveGameLoadingError: @"Not implemented for Apple Game Center"];
    } else {
        [self saveGameLoadingError: @"Not connected to Apple Game Center"];
    }
}

- (void)saveGameSave:(NSString*) saveGameName saveGameContents:(NSString*) saveGameContents description:(NSString*) description playedTimeMillis:(long) playedTimeMillis
{
    NSLog(@"TODO: Saving game not implemented");
}

- (bool)messageReceived:(NSArray* )message
{
    if (message.count == 3 &&
        [[message objectAtIndex: 0] isEqualToString:@"game-service-initialize"])
    {
        bool autoStartSignInFlowExplicit = [self stringToBool: [message objectAtIndex: 1]];
        bool saveGames = [self stringToBool: [message objectAtIndex: 2]];
        [self initialize: autoStartSignInFlowExplicit saveGames: saveGames];
        return TRUE;
    } else
    if ([message isEqualToArray: @[@"show", @"achievements"]])
    {
        [self showAchievements];
        return TRUE;
    } else
    if (message.count == 3 &&
        [[message objectAtIndex: 0] isEqualToString:@"show"] &&
        [[message objectAtIndex: 1] isEqualToString:@"leaderboard"])
    {
        //NSString* leaderboardId = [message objectAtIndex: 2]; // TODO: ignored
        [self showLeaderboards];
        return TRUE;
    } else
    if (message.count == 2 &&
        [[message objectAtIndex: 0] isEqualToString:@"achievement"])
    {
        NSString* achievementId = [message objectAtIndex: 1];
        [self achievement: achievementId];
        return TRUE;
    } else
    if (message.count == 2 &&
        [[message objectAtIndex: 0] isEqualToString:@"game-service-sign-in"])
    {
        bool wantsSignedIn = [self stringToBool: [message objectAtIndex: 1]];
        [self requestSignIn: wantsSignedIn];
        if (!wantsSignedIn) {
            [autoSignIn setAutoSignIn: false];
        }
        return TRUE;
    } else
    if (message.count == 2 &&
        [[message objectAtIndex: 0] isEqualToString:@"save-game-load"])
    {
        NSString* saveGameName = [message objectAtIndex: 1];
        [self saveGameLoad: saveGameName];
        return TRUE;
    } else
    if (message.count == 5 &&
        [[message objectAtIndex: 0] isEqualToString:@"save-game-save"])
    {
        NSString* saveGameName = [message objectAtIndex: 1];
        NSString* saveGameContents = [message objectAtIndex: 2];
        NSString* description = [message objectAtIndex: 3];
        long playedTimeMillis = [[message objectAtIndex: 4] longLongValue];
        [self saveGameSave: saveGameName saveGameContents: saveGameContents description: description playedTimeMillis: playedTimeMillis];
        return TRUE;
    }

    return FALSE;
}

@end
