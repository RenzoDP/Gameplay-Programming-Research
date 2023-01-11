# Flooding the Influence map for chase

## Description

Most stealth-based games in the current industry make use of an AI that seems to be all-knowing.
For example; I'm sure you ever came across the situation where you were sneaking around the enemies' base gathering intel,
until suddenly a guard came across you. You tried to get away, but the AI seamingly seemed to know your every move, making it rather impossible to get away.

That's what this algorithm is trying to avoid, while still creating a thrilling chase sequence.

This is created using a variant of the Influence map algorithm.
It's splits up the propagating influence between a positive influence your guard AI want's follow
and an influence one that blocks of the former one.

![ResultGif](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/Result.gif)

## Design

As stated before, the algorithm consists of two influences: a positive influence and a "blockade" influence.
The blockade influence is needed so that the positive influence won't spread behind the player's last known velocity-direction.
It normally wouldn't make sense for the player to run back towards the guard and if he does so, he would be caught anyways.
These then spread themselves over the map accordingly. 
To achieve this result, the algorithm has to take a few steps.


**The first step is the actual perceiving of the player and getting it's information**

The AI has to be able to see the player and get it's velocity-direction before any other part of the algorithm can work.
When the player leaves the AI's FOV, the algorithm can start.
The last known player's position and velocity-direction are stored for further use in the algorithm.

![PerceivingPlayer](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/Vision.png)

**The second step is putting up the first blockade and setting the influence**

To setup the first couple of blockade-nodes, it will have to get the node the player was last seen on.
All the nodes that are connected to it and are behind the player's last known velocity-direction will turn into a blockade-node.
After setting up the blockade, the player's last known node will emit a positive influence.

![SettingFirstBlockade](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/SetBlockade.png)

**The third step is propagating the blockade and the influence**

The positive influence emits itself in a wave-like manner.
All the nodes connected to a node that's on the max influence amount and aren't part of the blockade-nodes, emit the max influence amount.
All other nodes decrease their influence by a constant value.

The blockade-nodes on the other hand spread themselves out like a virus.
Every node that's connected to a blockade-node and is behind the player's last known velocity-direction, also become a blockade-node.

![PropagatingInfluence](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/InfluencePropagation.gif)

**The last and fourth step is limiting your algorithm**

This is needed so that the algorithm doesn't run forever.
There are a few conditions that stop the algorithm, them being:

- *Reaching the maximum amount of positive nodes in a propagation step*
This value controls how long the algorithm will run when it reaches an open space.
When the new set positive nodes exceeds the limit in a propagation step, the algorithm stops.

   ![LowHeatedCells](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/LowHeatedCells.gif)
  - Low maximum amount of positive nodes

   ![HighHeatedCells](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/HighHeatedCells.gif)
  - High maximum amount of positive nodes



* *Reaching the maximum amount of propagation steps*
This value controls how long the algorithm will run for alltogether.
The algorithm remembers how many steps it has set since it's beginning.
When the current propagation steps exceeds the max amount, the algorithm stops.

   ![LowPropagationSteps](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/LowPropagationSteps.gif)
  - Low maximum amount of propagation steps

   ![HighPropagationSteps](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/HighPropagationSteps.gif)
  - High maximum amount of propagation steps



+ *Having no more nodes to emit positive influence to left*
When the algorithm reaches a point where it can't spread it's positive influence anymore, it stops.


######  Adding other settings

It's also possible to add in more settings into your algorithm.
Some of the custom settings my version of the algorithm has are:


*Max Heat*

This value controls what the maximum influence is that a node can be.
It controls how long the positive nodes will linger after being put to max influence.

   ![LowMaxHeat](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/LowHeat.gif)
   - Low max heat
   
   ![HighMaxHeat](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/HighHeat.gif)
   - High max heat


*Propagation Interval*

This value controls how long it will take before another propagation step gets taken.

   ![LowPropagationInterval](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/LowPropagationInterval.gif)
   - Low propagation interval

   ![HighPropagationInterval](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/HighPropagationInterval.gif)
   - High propagation interval

## Result

The result of taking all the steps is an influence map that predicts where the player could have run off to,
after being perceived by the AI.

How the AI handles this data is totally up to the designer.
The AI could for example; follow the positive nodes towards the player,
it could throw an attack that envelops the area behind the player, ...
The possibilities are endless.

A thing the player could do to counter-act this behavior without getting caught, is simply hiding.
While the guard runs towards the area he thinks the player ran off to,
the player could be laughing his butt off watching the guard run past him while he's hiding behind a bush.

While allowing many fun experiences, the algorithm doesn't come without it's holes.

![Holes](https://github.com/RenzoDP/Gameplay-Programming-Research/blob/AddingGifsToReadMe/Gifs/Holes.gif)

## Conclusion

In conclusion, this chasing algorithm is great for when you want to replace your all-knowing AI with a more intuitive one.
While it's not really perfect for all situations, it does it's job at creating an enjoyable chasing sequence.

And after all, isn't having an AI that seemingly guesses the main fascination of Artificial Intelligence?


## References

[Game AI Pro - Online Edition 2021.](http://www.gameaipro.com/) Flooding the Influence Map for Chase in Dishonored 2, Laurent Couvidou
