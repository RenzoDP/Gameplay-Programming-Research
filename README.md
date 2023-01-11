<p style="text-align: center;"><strong><span style="font-size: 60px;">Flooding the Influence map for chase</span></strong></p>
<p style="text-align: left;"><br></p>
<ul>
    <li style="text-align: left;"><span style="font-size: 24px;">Description</span></li>
</ul>
<p>Most stealth-based games in the current industry make use of an AI that seems to be all-knowing.<br>For example; I&apos;m sure you ever came across the situation where you were sneaking around the enemies&apos; base gathering intel,<br>until suddenly a guard came across you. You tried to get away, but the AI seamingly seemed to know your every move, making it rather impossible to get away.</p>
<p>That&apos;s what this algorithm is trying to avoid, while still creating a thrilling chase sequence.</p>
<p>This is created using a variant of the Influence map algorithm.<br>It&apos;s splits up the propagating influence between a positive influence your guard AI want&apos;s follow<br>and an influence one that blocks of the former one.</p>
<p><video controls="" width="100%">
        <source src="https://media.giphy.com/media/f3hA0Emh7XfoS0Aknk/giphy.mp4" type="video/mp4"></video></p>
<p><br></p>
<ul>
    <li style="text-align: left;"><span style="font-size: 24px;">Design</span></li>
</ul>
<p><span style="font-size: 16px;">As stated before, the algorithm consists of two influences: a positive influence and a &quot;blockade&quot; influence.</span><br><span style="font-size: 16px;">These then spread themselves over the map accordingly.&nbsp;</span><br><span style="font-size: 16px;">To achieve this result, the algorithm has to take a few steps.</span></p>
<p><br></p>
<p><strong>The first step is the actual perceiving of the player and getting it&apos;s information</strong></p>
<p>The AI has to be able to see the player and get it&apos;s velocity-direction before any other part of the algorithm can work.<br>When the player leaves the AI&apos;s FOV, the algorithm can start.<br>The last known player&apos;s position and velocity-direction are stored for further use in the algorithm.</p>
<p><br></p>
<p><strong>The second step is putting up the first blockade and setting the influence<br></strong></p>
<p>To setup the first couple of blockade-nodes, it will have to get the node the player was last seen on.<br>All the nodes that are connected to it and are behind the player&apos;s last known velocity-direction will turn into a blockade-node.<br>After setting up the blockade, the player&apos;s last known node will emit a positive influence.</p>
