<script setup lang="ts">
import { inject, reactive, ref } from 'vue';
import { backend_key, current_user_key } from '@/keys';
import { computed } from '@vue/reactivity';
import { ElMessage } from 'element-plus';

let backend = inject(backend_key)!;
let username = ref("");
let current_user = inject(current_user_key)!;

let validate = computed(() => {
	return username.value && current_user.value;
});
let validate_modify = computed(() => {
	if (!user_data2.name || !user_data2.mailAddr) return false;
	if (!user_data1.username || user_data1.username != user_data2.username) return false;
	if (user_data1.name == user_data2.name && user_data1.mailAddr == user_data2.mailAddr && user_data1.privilege == user_data2.privilege)
		return false;
	return true;
});

let history = new Map<string, string>();

function send_request_query_profile() {
	if (!validate.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, "query");
	backend.send(`[${timestamp}] query_profile -c ${current_user.value} -u ${username.value}`);
}


interface User {
	username: string,
	mailAddr: string,
	name: string,
	privilege: number
}

let user_data1: User = reactive({ username: "", mailAddr: "", name: "", privilege: 0 });
let user_data2: User = reactive({ username: "", mailAddr: "", name: "", privilege: 0 });
let enable_modify = ref(false);

function send_request_modify_profile() {
	if (!validate.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, "modify");
	let cmd = `[${timestamp}] modify_profile -c ${current_user.value} -u ${username.value}`;
	if (user_data1.name != user_data2.name)
		cmd += ` -n ${user_data2.name}`;
	if (user_data1.mailAddr != user_data2.mailAddr)
		cmd += ` -m ${user_data2.mailAddr}`;
	if (user_data1.privilege != user_data2.privilege)
		cmd += ` -g ${user_data2.privilege}`;
	backend.send(cmd);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	if (str.slice(27, 29) == "-1") {
		ElMessage.error(`${history.get(timestamp)} failed.`);
		return;
	}
	ElMessage.success(`${history.get(timestamp)} success.`);
	let data = str.slice(27).split(" ");
	user_data1.username = data[0];
	user_data1.name = data[1];
	user_data1.mailAddr = data[2];
	user_data1.privilege = Number(data[3]);
	enable_modify.value = false;
});
</script>

<template>
	<h1>用户信息</h1>
	<div class="center">
		<div>
			<div>用户名</div><el-input v-model="username" minlength="1" maxlength="20" :show-word-limit=true></el-input>
		</div>

		<el-button type="primary" plain class="my-button" @click="send_request_query_profile"
			:disabled="!validate">查询</el-button>

		<el-switch v-model="enable_modify" active-text="编辑模式" inactive-text="查看模式"
			@change="Object.assign(user_data2, user_data1)"></el-switch>

		<el-descriptions title="用户信息" v-show="!enable_modify && user_data1.username" :border="true" :column="2">
			<el-descriptions-item label="用户名">{{ user_data1.username }}</el-descriptions-item>
			<el-descriptions-item label="姓名">{{ user_data1.name }}</el-descriptions-item>
			<el-descriptions-item label="邮箱">{{ user_data1.mailAddr }}</el-descriptions-item>
			<el-descriptions-item label="权限">{{ user_data1.privilege }}</el-descriptions-item>
		</el-descriptions>

		<div v-show="enable_modify && user_data1.username">
			<div>
				<div>邮箱</div><el-input v-model="user_data2.mailAddr" minlength="1" maxlength="30" :disabled="!enable_modify"
					:show-word-limit="true"></el-input>
			</div>
			<div>
				<div>真实名称</div><el-input v-model="user_data2.name" minlength="1" maxlength="5" :disabled="!enable_modify"
					:show-word-limit="true"></el-input>
			</div>
			<div>
				<div>权限</div><el-input-number v-model="user_data2.privilege" :disabled="!enable_modify" :min="1" :max="10"
					:precision="0" />
			</div>
			<el-button @click="send_request_modify_profile" :disabled="!validate_modify">保存更改</el-button>
		</div>

	</div>
</template>
