This is a project mostly for learning, and will probably take a very long time
and produce nothing of real worth in the end.

However, the main concept of this is to build a game by hand following some
tutorials related to the Handmade Hero video series along with other materials
from that community. Check it out: https://hero.handmade.network/

Very early on this will diverge from Handmade Hero because it's a completely
different type of game (for example, no joystick/gamepad). However, following 
Casey's methodologies and the lessons provided helps get it off the ground 
regardling platform layer, renderer, audio, etc. I highly recommend his series.

Additionally, I will not be fully adhering to Casey's non-use of libraries, since
eventually I want to hardware accelerate the rendering. And this game will have 
network code. Additionally, I'm utilizing SDL2 instead of writing directly for the
platform I'm developing on.

# Building

Toolchain is LLVM-19, so clang-19, etc etc
GNUMakefile, and use your favorite compilation database generator for clangd support. I use bear

# The goals/milestones for the game are:

## The player may join and own a corporation

The game mechanics should include both participating in an NPC corporation and
owning a player corporation

## Economic Model

The concept of the economics in the game will follow corporate, contractual
economics, instead of the kind of buy low sell high representation often seen.
Essentially, instead of buying wares at one factory and then selling them at
another, there is a contract for a transport corporation to move those goods.

## Mercenary Career

The player can choose to join or own a mercenary corporation instead of an
industrial one.

## NPC corporations

The universe should also have existing corporations that are behaving as
corporations ought to behave.

## Combat System

The basics of the combat system is a real-time turn-based rhythm game, where
attacks land on one of 4 beats per turn, and the defense systems need to be
timed appropriately.

## Multiplayer

The ability to host a game for up to a dozen players.
